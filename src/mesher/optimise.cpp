#include "optimise.h"
#include <Eigen/Core>
#include <random>
#include <iostream>
#include <RoSy/RoSy.h>
#include <sys/stat.h>

const int RANDOM_SEED = 474264642;

/**
 * Optimise the entire hierarchy. Do each level repeatedly until converged.
 */
void
Optimiser::optimise(std::vector<Surfel>& surfels) {
    using namespace std;

    if (is_optimising) {
        throw runtime_error("Already optimising");
    }

    optimise_begin(surfels);
    while (optimising_should_continue()) {
        optimise_do_one_step(surfels);
    }
    optimise_end();
}


/**
 * Check whether user asked for optimiseng to halt.
 */
bool
Optimiser::user_canceled_optimise() {
    struct stat buffer{};
    return (stat("halt", &buffer) == 0);
}


/**
 * Start global smoothing.
 */
void
Optimiser::optimise_begin(std::vector<Surfel>& surfels) {
    populate_frame_to_surfel(surfels);
    populate_norm_tan_by_surfel_frame(surfels);
    populate_neighbours_by_surfel_frame(surfels);
    last_optimising_error = compute_error(surfels);
    is_optimising = true;
}


/**
 * Perform post-smoothing tidy up.
 */
void
Optimiser::optimise_end() {
    is_optimising = false;
}

/**
 * Measure the change in error. If it's below some threshold, consider this level converged.
 */
bool
Optimiser::check_convergence(std::vector<Surfel>& surfels) {
    float latest_error = compute_error(surfels);
    float delta_error = fabsf(latest_error - last_optimising_error);
    last_optimising_error = latest_error;
    return (delta_error / last_optimising_error < convergence_threshold);
}

/**
 * Compute the erorr per surfel/per frame
 */
float
Optimiser::compute_error(std::vector<Surfel>& surfels) {
    using namespace std;
    using namespace Eigen;

    const size_t num_surfels = surfels.size();

    // Zero out the error.
    float error[num_surfels][num_frames];
    float surfel_error[num_surfels];
    float frame_error[num_frames];
    for (int s = 0; s < num_surfels; ++s) {
        for (int f = 0; f < num_frames; ++f) {
            error[s][f] = 0.0f;
        }
        surfel_error[s] = 0.0f;
    }
    for (float &f : frame_error) {
        f = 0.0f;
    }
    float total_error = 0.0f;

    // For each frame
    for (int f = 0; f < num_frames; ++f) {
        // Get surfels in frame
        const auto &surfels_in_this_frame = surfels_by_frame.at(f);

        // For each surfel in the frame, compute the error with each neighbour
        for (const auto &s : surfels_in_this_frame) {
            const auto surfel_in_frame = make_pair(s.get().id, f);
            const auto this_surfel_in_this_frame = norm_tan_by_surfel_frame.at(surfel_in_frame);

            const auto &neighbours_of_this_surfel = neighbours_by_surfel_frame.at(surfel_in_frame);

            float err = 0.0f;
            int count = 0;
            for (const auto &n :neighbours_of_this_surfel) {
                //TODO NB We can cache error computations here
                const auto this_neighbour_in_this_frame = norm_tan_by_surfel_frame.at(make_pair(n.get().id, f));

                // Compute the error between this surfel in this frame and the neighblur in this frame.
                err += compute_error(this_surfel_in_this_frame, this_neighbour_in_this_frame);
                count++;
            }
            err /= (float) count;
            error[s.get().id][f] += err;
            surfel_error[s.get().id] += err;
            frame_error[f] += err;
        }
    }
    return total_error;
}


// TODO: It feels like this is not a great error measure and we should rotate the two tangents into the same
// plane before computing the angle between them.
//
/**
 * Compute the error between two tangent vectors as the square of the angle between their 4RoSy rotations.
 *
 * @param first First normal/tangent pair.
 * @param second Second normal/tangent pair.
 * @return
 */
float
Optimiser::compute_error(const std::pair<Eigen::Vector3f, Eigen::Vector3f> &first,
                               const std::pair<Eigen::Vector3f, Eigen::Vector3f> &second) {
    using namespace std;
    using namespace Eigen;

    // parameter order in RoSy is tangent, normal
    pair<Vector3f, Vector3f> best_pair = best_rosy_vector_pair(first.second, first.first, second.second, second.first);
    float theta = angle_between_vectors(best_pair.first, best_pair.second);
    return (theta * theta);
}

/**
 * Check whether optimising should stop either because user asked for that to happen or else
 * because convergence has happened.
 */
bool
Optimiser::optimising_should_continue() {
    return (user_canceled_optimise() || optimising_converged);
}

/**
 * Perform a single step of optimisation.
 */
void
Optimiser::optimise_do_one_step(std::vector<Surfel>& surfels) {
    using namespace std;

    optimise_do_one_surfel(surfels);
    check_convergence(surfels);
}

/**
 * On entry surfel and neighbour frames are sorted in ascending order of frame_id
 * common_frames is allocated. The algorithm will clear and resize it.
 * on exit, common_frames contains pairs of <surfel,neghbour> frames with matching IDs
 */
void
find_common_frames(const std::vector<FrameData> &surfel_frames,
                   const std::vector<FrameData> &neighbour_frames,
                   std::vector<std::pair<FrameData, FrameData>> &common_frames) {
    using namespace std;

    unsigned long max_common_values = min(surfel_frames.size(), neighbour_frames.size());
    common_frames.clear();
    common_frames.resize(max_common_values);

    // Find the intersection of these sets
    auto surfel_frame_iter = surfel_frames.begin();
    auto neighbour_frame_iter = neighbour_frames.begin();
    auto common_iter = common_frames.begin();

    while (surfel_frame_iter != surfel_frames.end() && neighbour_frame_iter != neighbour_frames.end()) {
        if (surfel_frame_iter->pixel_in_frame.frame < neighbour_frame_iter->pixel_in_frame.frame) {
            ++surfel_frame_iter;
        } else if (neighbour_frame_iter->pixel_in_frame.frame < surfel_frame_iter->pixel_in_frame.frame) {
            ++neighbour_frame_iter;
        } else {
            *common_iter = make_pair(*surfel_frame_iter, *neighbour_frame_iter);
            ++common_iter;
            ++surfel_frame_iter;
            ++neighbour_frame_iter;
        }
    }
    common_frames.resize(common_iter - common_frames.begin());
}

/**
 * Populate tangents and normals with all eligible tangents, normals from surfel's neighbours
 * tan/norm is eligible iff the neighbour and surfel share a common frame
 * tan/norm are converted to the orignating surfel's frame of reference.
 */
void
Optimiser::get_eligible_normals_and_tangents(const std::vector<Surfel>& surfels,
        std::size_t surfel_idx,
                                                   std::vector<Eigen::Vector3f> &eligible_normals,
                                                   std::vector<Eigen::Vector3f> &eligible_tangents) const {
    using namespace std;
    using namespace Eigen;

    const vector<FrameData> &surfel_frames = surfels.at(surfel_idx).frame_data;

    // For each neighbour
    int total_common_frames = 0;
    for (size_t neighbour_idx : surfels.at(surfel_idx).neighbouring_surfels) {

        const vector<FrameData> &neighbour_frames = surfels.at(neighbour_idx).frame_data;
        vector<pair<FrameData, FrameData>> common_frames;
        find_common_frames(surfel_frames, neighbour_frames, common_frames);
        total_common_frames += common_frames.size();

        // For each common frame, get normal and tangent in surfel space
        for (auto const &frame_pair : common_frames) {
            const Matrix3f surfel_to_frame = frame_pair.first.transform;
            const Matrix3f frame_to_surfel = surfel_to_frame.transpose();
            const Matrix3f neighbour_to_frame = frame_pair.second.transform;
            Vector3f neighbour_normal_in_frame = frame_pair.second.normal;

            // Push the neighbour normal and tangent into the right frame
            // Transform the frame tangent back to the surfel space using inv. surfel matrix
            // So we need:
            //    transform from free space to frame space for tangent (stored) (we already have normal in frame space)
            //	  transform from frame space to free space using surfel data. (inv of stored)
            const Matrix3f neighbour_to_surfel = frame_to_surfel * neighbour_to_frame;
            Vector3f neighbour_tan_in_surfel_space =
                    neighbour_to_surfel * surfels.at(neighbour_idx).tangent;
            Vector3f neighbour_norm_in_surfel_space = frame_to_surfel * neighbour_normal_in_frame;
            eligible_normals.push_back(neighbour_norm_in_surfel_space);
            eligible_tangents.push_back(neighbour_tan_in_surfel_space);
        }
    }
    // cout << "  total common frames " << total_common_frames << endl;
}

/**
 * Compute a new tangent for the given surfel S
 *
 * for each neighbouring surfel N of S
 *   find any frame f in which both N and S are visible
 *   obtain MfS the transformation matrix from frame f for S
 *   obtain dirNS from MfS * dirN
 *   perform 4RoSy smoothing operation on dirS and dirNS
 * end
 */
Eigen::Vector3f
Optimiser::compute_new_tangent_for_surfel(const std::vector<Surfel>& surfels, std::size_t surfel_idx) const {
    using namespace Eigen;
    using namespace std;

    // Get vector of eligible normal/tangent pairs
    vector<Vector3f> normals;
    vector<Vector3f> tangents;
    get_eligible_normals_and_tangents(surfels, surfel_idx, normals, tangents);

    // Merge all neighbours; implicitly using optiminsing tier tangents
    Vector3f new_tangent = surfels.at(surfel_idx).tangent;

    float weight = 0;
    for (size_t idx = 0; idx < normals.size(); ++idx) {
        float edge_weight = 1.0f;
        new_tangent = average_rosy_vectors(new_tangent, Vector3f{0.0f, 1.0, 0.0f}, weight,
                                           tangents.at(idx), normals.at(idx), edge_weight);
        weight += edge_weight;
    }
    return new_tangent;
}

void
Optimiser::optimise_do_one_surfel(std::vector<Surfel>& surfels) {
    using namespace std;
    using namespace Eigen;

    // Select random surfel
    size_t surfel_idx = random_index(surfels.size());

    // Update this one
    Vector3f new_tangent = compute_new_tangent_for_surfel(surfels, surfel_idx);
    surfels.at(surfel_idx).tangent = new_tangent;
}

/**
 * Select a random integer in the range [0, max_index)
 */
std::size_t
Optimiser::random_index(unsigned int max_index) {
    static std::default_random_engine e{RANDOM_SEED};
    static std::uniform_real_distribution<> dis(0, max_index);
    return std::floor(dis(e));
}

/**
 * Build the norm_tan_by_surfel_frame data structure for this level of the
 * surfel hierarchy. Note that tans will be updated every optimisation
 * We should calculat this once per level and then update each time we change a tan.
 */
void
Optimiser::populate_norm_tan_by_surfel_frame(std::vector<Surfel>& surfels) {
    using namespace std;
    using namespace Eigen;

    norm_tan_by_surfel_frame.clear();

    /*
       For each surfel
       For each framedata in the surfel
       Generate a surfel-frame pair
       transform the norm and tan
       put it in the map.
     */
    for (const auto &surfel : surfels) {
        for (const auto &fd : surfel.frame_data) {
            pair<size_t, size_t> surfel_in_frame = make_pair(surfel.id, fd.pixel_in_frame.frame);
            Vector3f new_norm = fd.normal;
            Vector3f new_tan = fd.transform * surfel.tangent;
            norm_tan_by_surfel_frame.insert(make_pair(surfel_in_frame, make_pair(new_norm, new_tan)));
        }
    }
}

/**
 * Populate the surfels per fram map.
 */
void
Optimiser::populate_frame_to_surfel(std::vector<Surfel>& surfels) {
    using namespace std;
    assert(num_frames > 0);
    /*
      For each surfel
      For each frame
      add the surfel to the frame.
     */
    surfels_by_frame.clear();
    for (int i = 0; i < num_frames; ++i) {
        vector<reference_wrapper<const Surfel>> empty;
        surfels_by_frame.push_back(empty);
    }

    for (const auto &surfel : surfels) {
        for (const auto &fd : surfel.frame_data) {
            surfels_by_frame.at(fd.pixel_in_frame.frame).push_back(surfel);
        }
    }
}

/**
 * Build the neighbours_by_surfel_frame data structure. Neighbours stay neighbours throughout and so we can compute this
 * We assume that
 * -- neighbours by surfel frame is empty
 * -- surfels_by_frame is populated for this level
 *
 * But num_frames and num_surfels are both known.
 */
void
Optimiser::populate_neighbours_by_surfel_frame(const std::vector<Surfel>& surfels) {
    using namespace std;

    assert(!surfels_by_frame.empty());

    /*
      Build a map from frame to surfel const ref
      For each surfel
      For each frame data
      Get a vector of surfels in that frame (from map)
      Get a vector of neighbouring surfels (from surfel)
      Sort both by surfel index
      iterate across both finding common elements
      For each common element, add an entry to the map.
     */
    for (const auto &surfel : surfels) {
        auto neighbours_of_this_surfel = surfel.neighbouring_surfels;
        for (const auto &fd : surfel.frame_data) {
            size_t frame = fd.pixel_in_frame.frame;
            vector<reference_wrapper<const Surfel>> surfels_in_this_frame = surfels_by_frame.at(frame);
            vector<reference_wrapper<const Surfel>> neighbours_in_this_frame;
            compute_intersection_of(neighbours_of_this_surfel, surfels_in_this_frame, neighbours_in_this_frame);
            neighbours_by_surfel_frame.insert( //
                    make_pair( //
                            make_pair(surfel.id, frame),
                            neighbours_in_this_frame));
        }
    }
}

/**
 * Compute the intersection of the two provided vectors and place the results into the third.
 */
void
Optimiser::compute_intersection_of(std::vector<size_t> neighbours_of_this_surfel,
                                         std::vector<std::reference_wrapper<const Surfel>> surfels_in_this_frame,
                                         std::vector<std::reference_wrapper<const Surfel>> neighbours_in_this_frame) {
    using namespace std;

    // Sort neighbours of tis surfel
    sort(neighbours_of_this_surfel.begin(),
         neighbours_of_this_surfel.end(),
         [](size_t s1, size_t s2) {
             return s1 < s2;
         });

    sort(surfels_in_this_frame.begin(),
         surfels_in_this_frame.end(),
         [](const Surfel &s1, const Surfel &s2) {
             return s1.id < s2.id;
         });

    // Iterate across them both
    auto it1 = neighbours_of_this_surfel.begin();
    auto it2 = surfels_in_this_frame.begin();
    while (it1 != neighbours_of_this_surfel.end() && it2 != surfels_in_this_frame.end()) {
        size_t s1 = *it1;
        const Surfel &surfel = *it2;
        size_t s2 = surfel.id;

        if (s1 < s2) {
            it1++;
        } else if (s2 < s1) {
            it2++;
        } else {
            neighbours_in_this_frame.emplace_back(surfel);
            it1++;
            it2++;
        }
    }
}