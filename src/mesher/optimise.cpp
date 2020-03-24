#include "optimise.h"
#include <Eigen/Core>
#include <random>
#include <iostream>
#include <RoSy/RoSy.h>
#include <sys/stat.h>

static std::random_device r_device;
static std::default_random_engine r_engine(r_device());

/**
 * Optimise the vector of surfels
 */
int
Optimiser::optimise(std::vector<Surfel> &surfels) {
    using namespace std;

    int optimisation_cycles = 0;

    if (is_optimising) {
        throw runtime_error("Already optimising");
    }

    optimise_begin(surfels);
    while (optimising_should_continue()) {
        optimise_do_one_step(surfels);
        // Then, project each Surfel's norm and tangent to each frame in which it appears
        populate_norm_tan_by_surfel_frame(surfels);
        optimising_converged = check_convergence(surfels);
        ++optimisation_cycles;
    }
    optimise_end();
    return optimisation_cycles;
}


/**
 * Check whether user asked for optimiseng to halt.
 */
bool
Optimiser::user_canceled_optimise() {
    struct stat buffer{};
    auto rv = stat("halt", &buffer);
    return (rv == 0);
}


/**
 * Start global smoothing.
 */
void
Optimiser::optimise_begin(const std::vector<Surfel> &surfels) {
    using namespace std;

    is_optimising = true;

    // First we construct a mapping from frame to surfel
    cout << "Mapping frames to surfels" << endl;
    populate_frame_to_surfel(surfels);

    // Then, project each Surfel's norm and tangent to each frame in which it appears
    cout << "Mapping surfel-frame to normal/tan" << endl;
    populate_norm_tan_by_surfel_frame(surfels);

    // Populate map of neighbours for a surfel in a frame
    cout << "Computing neighbours for each surfel-frame" << endl;
    populate_neighbours_by_surfel_frame(surfels);

    // Compute initial error values
    cout << "Initialising error value" << endl;
    last_optimising_error = compute_mean_error_per_surfel(surfels);
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
Optimiser::check_convergence(std::vector<Surfel> &surfels) {
    float latest_error = compute_mean_error_per_surfel(surfels);
    float improvement = last_optimising_error - latest_error;
    last_optimising_error = latest_error;
    float pct = (100.0f * improvement) / last_optimising_error;
    std::cout << "Mean error per surfel : " << latest_error << ". %age reduction " << pct << std::endl;
    return (std::abs(pct) < convergence_threshold);
}


/*
 * total_neighbour_error = 0
 * for each neighbour
 *   total_neighbour_error += neighour_error
 * next
 * return total_neighbour_error / num neighours
 */
float
Optimiser::compute_surfel_error_for_frame(const std::string & surfel_id, size_t frame_id) {
    float total_neighbour_error = 0.0f;

    // Get all neighbours in frame
    const SurfelInFrame surfel_in_frame{surfel_id, frame_id};

    const auto& this_surfel_in_this_frame = norm_tan_by_surfel_frame.at(surfel_in_frame);
    const auto& neighbours_of_this_surfel = neighbours_by_surfel_frame.at(surfel_in_frame);
    for (const auto &n : neighbours_of_this_surfel) {
        const auto& this_neighbour_in_this_frame = norm_tan_by_surfel_frame.at(SurfelInFrame{n, frame_id});

        // Compute the error between this surfel in this frame and the neighbour in this frame.
        total_neighbour_error += compute_error(this_surfel_in_this_frame, this_neighbour_in_this_frame);
    }
    return (neighbours_of_this_surfel.empty())
           ? 0.0f
           : (total_neighbour_error / neighbours_of_this_surfel.size());
}


/*
 * total_frame_error = 0
 * for each frame
 *   total_frame_error += compute_surfel_error_in_frame
 * next
 * total_surfel_error += (total_frame_error / num_frames)
 */
float
Optimiser::compute_surfel_error(const Surfel &surfel) {
    float total_frame_error = 0.0f;

    // For each frame in which this surfel appears
    for (const auto &frame_data : surfel.frame_data) {
        // Compute the error in this frame
        total_frame_error += compute_surfel_error_for_frame(surfel.id, frame_data.pixel_in_frame.frame);
    }
    // Return mean error per frame
    return total_frame_error / surfel.frame_data.size();
}

/*
    err = 0
    for each surfel
      err += surfel error
    next
    err = surfel_error / num surfels
    */
float
Optimiser::compute_mean_error_per_surfel(const std::vector<Surfel> &surfels) {

    float total_error = 0.0f;
    for (const auto &surfel : surfels) {
        total_error += compute_surfel_error(surfel);
    }
    return total_error / surfels.size();
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
Optimiser::compute_error(const NormalTangent& first,
                         const NormalTangent& second) {
    using namespace std;
    using namespace Eigen;

    // parameter order in RoSy is tangent, normal
    pair<Vector3f, Vector3f> best_pair = best_rosy_vector_pair(first.tangent, first.normal, second.tangent, second.normal);
    float theta = angle_between_vectors(best_pair.first, best_pair.second);
    return (theta * theta);
}

/**
 * Check whether optimising should stop either because user asked for that to happen or else
 * because convergence has happened.
 */
bool
Optimiser::optimising_should_continue() {
    return (!user_canceled_optimise() && !optimising_converged);
}

/**
 * Perform a single step of optimisation.
 */
void
Optimiser::optimise_do_one_step(std::vector<Surfel> &surfels) {
    using namespace std;

    for (int i = 0; i < surfels_per_step; ++i) {
        // Select random surfel
        size_t surfel_idx = random_index(surfels.size());

        // Smooth the selected surfel frame
        optimise_surfel(surfels, surfel_idx);
    }
}

/**
 * Given a source Normal and Tangent pair and a list of neighbours
 * compute a new tangent which is smoothed to neighbours
 */
Eigen::Vector3f
compute_smoothed_tangent(const NormalTangent &source,
                         const std::vector<NormalTangent> &neighbours) {
    using namespace Eigen;

    // Merge all neighbours; implicitly using optiminsing tier tangents
    Vector3f new_tangent = source.tangent;

    float weight = 0;
    for (const auto &nt : neighbours) {
        float edge_weight = 1.0f;
        new_tangent = average_rosy_vectors(new_tangent, source.normal, weight, nt.tangent, nt.normal, edge_weight);
        weight += edge_weight;
    }
    return new_tangent;
}

/**
 * Perform smoothing for a single surfel in a single frame
 * @param surfel_idx The index of the surfel WITHIN the vector
 * @param frame_idx The index of the frame WITHIN the surfel's frame_data
 */
void
Optimiser::smooth_surfel_in_frame(std::vector<Surfel> &surfels, size_t surfel_idx, size_t frame_idx) {
    using namespace std;
    using namespace Eigen;

    Surfel &surfel = surfels.at(surfel_idx);
    const FrameData &frame_data = surfel.frame_data.at(frame_idx);
    const Matrix3f &transformation_matrix = frame_data.transform;
    size_t frame_id = frame_data.pixel_in_frame.frame;

    // Get the normal and tangent of the source surfel in the specified frame
    NormalTangent source{frame_data.normal, transformation_matrix * surfel.tangent};
    SurfelInFrame sif{surfel.id, frame_id};

    const auto &neighbours_of_this_surfel = neighbours_by_surfel_frame.at(sif);
    vector<NormalTangent> neighbour_norm_tans;
    for (const auto &neighbour : neighbours_of_this_surfel) {
        const auto &nt = norm_tan_by_surfel_frame.at(SurfelInFrame{neighbour, frame_id});
        neighbour_norm_tans.push_back(nt);
    }

    Vector3f new_tangent = compute_smoothed_tangent(source, neighbour_norm_tans);
    surfels.at(surfel_idx).tangent = new_tangent;
}

/**
 * Return a vector of pairs of framedata for each frame that these surfels have in common
 */
std::vector<std::pair<std::reference_wrapper<const FrameData>, std::reference_wrapper<const FrameData>>>
find_common_frames_for_surfels( const Surfel& surfel1, const Surfel& surfel2 ) {
    using namespace std;

    vector<reference_wrapper<const FrameData>> surfel1_frames;
    vector<reference_wrapper<const FrameData>> surfel2_frames;
    for( const auto & f : surfel1.frame_data) {
        surfel1_frames.emplace_back(f);
    }
    for( const auto & f : surfel2.frame_data) {
        surfel2_frames.emplace_back(f);
    }
    sort(surfel1_frames.begin(), surfel1_frames.end(), [](const FrameData& f1, const FrameData& f2){return f1.pixel_in_frame.frame < f2.pixel_in_frame.frame;});
    sort(surfel2_frames.begin(), surfel2_frames.end(), [](const FrameData& f1, const FrameData& f2){return f1.pixel_in_frame.frame < f2.pixel_in_frame.frame;});

    vector<pair<reference_wrapper<const FrameData>, reference_wrapper<const FrameData>>> common_frames;
    auto it1 = surfel1.frame_data.begin();
    auto it2 = surfel2.frame_data.begin();
    while(it1 != surfel1.frame_data.end() && it2 != surfel2.frame_data.end() ) {
        if( it1->pixel_in_frame.frame < it2->pixel_in_frame.frame) {
            ++it1;
        } else if ( it2->pixel_in_frame.frame < it1->pixel_in_frame.frame) {
            ++it2;
        } else {
            common_frames.emplace_back(ref(*it1), ref(*it2));
            ++it1;
            ++it2;
        }
    }
    return common_frames;
}

/**
 * Populate tangents and normals with all eligible tangents, normals from surfel's neighbours
 * tan/norm is eligible iff the neighbour and surfel share a common frame
 * tan/norm are converted to the orignating surfel's frame of reference.
 */
std::vector<NormalTangent>
Optimiser::get_eligible_normals_and_tangents(const std::vector<Surfel> &surfels, std::size_t surfel_idx) const {
    using namespace std;
    using namespace Eigen;

    std::vector<NormalTangent> eligible_normals_and_tangents;

    const Surfel& this_surfel = surfels.at(surfel_idx);

    // For each neighbour
    int total_common_frames = 0;
    for (const auto& that_surfel_id : this_surfel.neighbouring_surfels) {
        const Surfel& that_surfel = Surfel::surfel_for_id(that_surfel_id);

        auto common_frames = find_common_frames_for_surfels(this_surfel, that_surfel);
        total_common_frames += common_frames.size();

        // For each common frame, get normal and tangent in surfel space
        for (auto const &frame_pair : common_frames) {
            const Matrix3f surfel_to_frame = frame_pair.first.get().transform;
            const Matrix3f frame_to_surfel = surfel_to_frame.transpose();
            const Matrix3f neighbour_to_frame = frame_pair.second.get().transform;
            Vector3f neighbour_normal_in_frame = frame_pair.second.get().normal;

            // Push the neighbour normal and tangent into the right frame
            // Transform the frame tangent back to the surfel space using inv. surfel matrix
            // So we need:
            //    transform from free space to frame space for tangent (stored) (we already have normal in frame space)
            //	  transform from frame space to free space using surfel data. (inv of stored)
            const Matrix3f neighbour_to_surfel = frame_to_surfel * neighbour_to_frame;
            Vector3f neighbour_tan_in_surfel_space = neighbour_to_surfel * that_surfel.tangent;
            Vector3f neighbour_norm_in_surfel_space = frame_to_surfel * neighbour_normal_in_frame;

            eligible_normals_and_tangents.emplace_back(neighbour_norm_in_surfel_space, neighbour_tan_in_surfel_space);
        }
    }
    // cout << "  total common frames " << total_common_frames << endl;
    return eligible_normals_and_tangents;
}

/*
  for each neighbouring surfel N of S
    find any frame f in which both N and S are visible
    obtain MfS the transformation matrix from frame f for S
    obtain dirNS from MfS * dirN
    perform 4RoSy smoothing operation on dirS and dirNS
  end
 */
void
Optimiser::optimise_surfel(std::vector<Surfel>& surfels, size_t surfel_idx) {
    using namespace std;
    using namespace Eigen;

    Surfel& surfel = surfels.at(surfel_idx);

    // Get vector of eligible normal/tangent pairs
    vector<NormalTangent> neighbouring_normals_and_tangents=get_eligible_normals_and_tangents(surfels, surfel_idx);

    // Merge all neighbours; implicitly using optiminsing tier tangents
    Vector3f new_tangent = surfel.tangent;

    float weight = 0;
    for (const auto & nt : neighbouring_normals_and_tangents) {
        float edge_weight = 1.0f;
        new_tangent = average_rosy_vectors(new_tangent, Vector3f{0.0f, 1.0, 0.0f}, weight,nt.tangent, nt.normal, edge_weight);
        weight += edge_weight;
    }

    surfel.tangent = new_tangent;
}

/**
 * Select a random integer in the range [0, max_index)
 */
std::size_t
Optimiser::random_index(unsigned int max_index) {
    assert(max_index >= 1);
    if (max_index == 1) return 0;

    // Seed with a real random value, if available
    std::uniform_int_distribution<int> uniform_dist(0, max_index - 1);
    return uniform_dist(r_engine);
}

/**
 * Build the norm_tan_by_surfel_frame data structure for this level of the
 * surfel hierarchy. Note that tans will be updated every optimisation
 * We should calculate this once per level and then update each time we change a tan.
 */
void
Optimiser::populate_norm_tan_by_surfel_frame(const std::vector<Surfel> &surfels) {
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
            SurfelInFrame surfel_in_frame{surfel.id, fd.pixel_in_frame.frame};
            Vector3f new_norm = fd.normal;
            Vector3f new_tan = fd.transform * surfel.tangent;
            norm_tan_by_surfel_frame.emplace(surfel_in_frame, NormalTangent{new_norm, new_tan});
        }
    }
}

/**
 * Build a mapping from frame to the Surfels which appear in that frame.
 * This allows us to work with frames individually.
 * surfels_by_frame is a member variable.
 */
void
Optimiser::populate_frame_to_surfel(const std::vector<Surfel> &surfels) {
    using namespace std;
    assert(num_frames > 0);

    // Push an empty vector of Surfel references for each frame
    surfels_by_frame.clear();
    for (int frame_index = 0; frame_index < num_frames; ++frame_index) {
        surfels_by_frame.emplace_back();
    }

    // For each Surfel add it to each frame in which it appears.
    for (const auto &surfel : surfels) {
        for (const auto &fd : surfel.frame_data) {
            surfels_by_frame.at(fd.pixel_in_frame.frame).push_back(surfel.id);
        }
    }


}

/**
 * Build the neighbours_by_surfel_frame data structure. Neighbours stay neighbours throughout and so we can compute this once
 * We assume that
 * -- surfels_by_frame is populated for this level
 *
 * But num_frames and num_surfels are both known.
 */
void
Optimiser::populate_neighbours_by_surfel_frame(const std::vector<Surfel> &surfels) {
    using namespace std;

    assert(!surfels_by_frame.empty());
    neighbours_by_surfel_frame.clear();

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
        const auto & neighbours_of_this_surfel = surfel.neighbouring_surfels;
        for (const auto &fd : surfel.frame_data) {
            size_t frame = fd.pixel_in_frame.frame;

            const auto & surfels_in_this_frame = surfels_by_frame.at(frame);
            auto neighbours_in_this_frame = compute_intersection_of(neighbours_of_this_surfel, surfels_in_this_frame);
            neighbours_by_surfel_frame.emplace(SurfelInFrame{surfel.id, frame}, neighbours_in_this_frame);
        }
    }
}

/**
 * Compute the intersection of the two provided vectors and place the results into the third.
 * Assumes that the vectors are sorted
 */
std::vector<std::string>
compute_intersection_of(std::vector<std::string> neighbours_of_this_surfel,
                                   std::vector<std::string> surfels_in_this_frame) {
    using namespace std;

    sort(surfels_in_this_frame.begin(), surfels_in_this_frame.end());

    std::vector<std::string> neighbours_in_this_frame(min(surfels_in_this_frame.size(), neighbours_of_this_surfel.size()));
    auto it = set_intersection(neighbours_of_this_surfel.begin(),
                               neighbours_of_this_surfel.end(),
                               surfels_in_this_frame.begin(),
                               surfels_in_this_frame.end(),
                               neighbours_in_this_frame.begin());
    neighbours_in_this_frame.resize(it - neighbours_in_this_frame.begin());
    return neighbours_in_this_frame;
}