#include "optimise.h"
#include <Eigen/Core>
#include <random>
#include <iostream>
#include <RoSy/RoSy.h>
#include <sys/stat.h>

const int RANDOM_SEED = 474264642;

static std::random_device r_device;
static std::default_random_engine r_engine(r_device());


/**
 * Optimise the vector of surfels
 */
void
Optimiser::optimise(std::vector<Surfel> &surfels) {
    using namespace std;

    if (is_optimising) {
        throw runtime_error("Already optimising");
    }

    optimise_begin(surfels);
    while (optimising_should_continue()) {
        optimise_do_one_step(surfels);
        // Then, project each Surfel's norm and tangent to each frame in which it appears
        populate_norm_tan_by_surfel_frame(surfels);
        optimising_converged = check_convergence(surfels);
    }
    optimise_end();
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
    // First we construct a mapping from frame to surfel
    populate_frame_to_surfel(surfels);
    // Then, project each Surfel's norm and tangent to each frame in which it appears
    populate_norm_tan_by_surfel_frame(surfels);
    // Populate map of neighbours for a surfel in a frame
    populate_neighbours_by_surfel_frame(surfels);
    // Compute initial error values
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
Optimiser::check_convergence(std::vector<Surfel> &surfels) {
    float latest_error = compute_error(surfels);
    float delta_error = fabsf(latest_error - last_optimising_error);
    last_optimising_error = latest_error;
    float pct =  delta_error / last_optimising_error;
    std::cout << "Mean error per surfel : " << latest_error << ". %age reduction " << pct*100 << std::endl;
    return (pct < convergence_threshold);
}

/**
 * Compute the erorr per surfel/per frame
 */
float
Optimiser::compute_error(const std::vector<Surfel> &surfels) {
    using namespace std;
    using namespace Eigen;

    float total_error = 0.0f;

    // For each frame
    for (size_t frame_idx = 0; frame_idx < num_frames; ++frame_idx) {
        // For each surfel in this frame, compute the error with its neighbours
        const auto &surfels_in_this_frame = surfels_by_frame.at(frame_idx);
        for (const auto &surfel : surfels_in_this_frame) {
            const SurfelInFrame surfel_in_frame{surfel.get().id, frame_idx};
            const auto this_surfel_in_this_frame = norm_tan_by_surfel_frame.at(surfel_in_frame);

            const auto &neighbours_of_this_surfel = neighbours_by_surfel_frame.at(surfel_in_frame);

            float surfel_error = 0.0f;
            for (const auto &n : neighbours_of_this_surfel) {
                //TODO NB We can cache error computations here
                const auto this_neighbour_in_this_frame = norm_tan_by_surfel_frame.at( SurfelInFrame{n.get().id, frame_idx});

                // Compute the error between this surfel in this frame and the neighbour in this frame.
                surfel_error += compute_error(this_surfel_in_this_frame, this_neighbour_in_this_frame);
            }
            surfel_error = (neighbours_of_this_surfel.empty()) ? 0.0f : (surfel_error / neighbours_of_this_surfel.size());
            total_error += surfel_error;
        }
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
    return (!user_canceled_optimise() && !optimising_converged);
}

/**
 * Perform a single step of optimisation.
 */
void
Optimiser::optimise_do_one_step(std::vector<Surfel> &surfels) {
    using namespace std;

    for( int i=0; i<surfels_per_step; ++i ) {
        optimise_one_surfel_frame(surfels);
    }
}

/**
 * Given a source Normal and Tangent pair and a list of neighbours
 * compute a new tangent which is smoothed to neighbours
 */
Eigen::Vector3f
compute_smoothed_tangent(const NormalTangent& source, const std::vector<NormalTangent>& neighbours ) {
    using namespace Eigen;

    // Merge all neighbours; implicitly using optiminsing tier tangents
    Vector3f new_tangent = source.tangent;

    float weight = 0;
    for (const auto& nt : neighbours) {
        float edge_weight = 1.0f;
        new_tangent = average_rosy_vectors(new_tangent, source.normal, weight, nt.tangent, nt.normal, edge_weight);
        weight += edge_weight;
    }
    return new_tangent;
}

/**
 * Get normals and tangents for neighbours of surfel in frame
 *
 * There are a couple of ways of achieving this.
 * The first:
 *    forward project surfel into given frame
 *    find all other surfels in that frame
 *    forward project their normals and tangents into thst frame using MAIN transform
 *    merge
 * i.e mean = 1/N sum(M_main * tan_i)_i=1..N
 *
 * Second:
 *    forward project surfel into given frame
 *    find all other surfels in that frame
 *    forward project their normals and tangents into thst frame using OWN transform
 *    merge
 *
 * Third:
 *    find all other surfels in frame
 *    forward project their normals and tangents into thst frame using OWN transform
 *    back project their normals and tangents into shared space using inverse of main transform
 *    merge
 *
 * The third is equivalent to second.
 * First is what we'll implement
 */
std::vector<NormalTangent>
get_norms_and_tans_for_surfel_neighbours_in_frame(const Eigen::Matrix3f &transform,
                                                             const std::vector<std::reference_wrapper<const Surfel>> &neighbour_surfels,
                                                             size_t surfel_id) {
    using namespace std;
    using namespace Eigen;

    // Get list of neighbouring surfels in this frame
    vector<NormalTangent> neighbours;
    for( const auto& neighbour_surfel : neighbour_surfels ) {
        if( neighbour_surfel.get().id != surfel_id ) {
            Vector3f normal  = transform * Vector3f{0,1.0,0};
            Vector3f tangent = transform * neighbour_surfel.get().tangent;
            neighbours.emplace_back(normal, tangent);
        }
    }
    return neighbours;
}

/**
 * Perform smoothing for a single surfel in a single frame
 * @param surfel_idx The index of the surfel WITHIN the vector
 * @param frame_idx The index of the frame WITHIN the surfel's frame_data
 */
void
Optimiser::smooth_surfel_in_frame(std::vector<Surfel>& surfels, size_t surfel_idx, size_t frame_idx ) {
    using namespace std;
    using namespace Eigen;

    Surfel& surfel = surfels.at(surfel_idx);
    const FrameData& frame_data = surfel.frame_data.at(frame_idx);
    const Matrix3f& transformation_matrix =  frame_data.transform;
    size_t frame_id = frame_data.pixel_in_frame.frame;

    // Get the normal and tangent of the source surfel in the specified frame
    NormalTangent source {frame_data.normal, transformation_matrix * surfel.tangent };
    SurfelInFrame sif{surfel.id, frame_id};
    vector<NormalTangent> neighbours = get_norms_and_tans_for_surfel_neighbours_in_frame(
            transformation_matrix,
            neighbours_by_surfel_frame.at(sif),
            surfel.id);

    Vector3f new_tangent = compute_smoothed_tangent(source, neighbours);
    surfels.at(surfel_idx).tangent = new_tangent;
}

/**
 * Pick a random surfel frame and smooth it with respect to its neighbours
 */
void
Optimiser::optimise_one_surfel_frame(std::vector<Surfel> &surfels) {
    using namespace std;
    using namespace Eigen;

    // Select random surfel
    size_t surfel_idx = random_index(surfels.size());

    // And pick a random frame
    size_t frame_idx = random_index(surfels.at(surfel_idx).frame_data.size());

    // Smooth the selected surfel frame
    smooth_surfel_in_frame(surfels, surfel_idx, frame_idx);
}

/**
 * Select a random integer in the range [0, max_index)
 */
std::size_t
Optimiser::random_index(unsigned int max_index) {
    assert( max_index >= 1);
    if( max_index == 1 ) return 0;

    // Seed with a real random value, if available
    std::uniform_int_distribution<int> uniform_dist(0, max_index-1);
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
            norm_tan_by_surfel_frame.insert(make_pair(surfel_in_frame, make_pair(new_norm, new_tan)));
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
        vector<reference_wrapper<const Surfel>> empty;
        surfels_by_frame.push_back(empty);
    }

    // For each Surfel add it to each frame in which it appears.
    for (const auto &surfel : surfels) {
        for (const auto &fd : surfel.frame_data) {
            surfels_by_frame.at(fd.pixel_in_frame.frame).push_back(surfel);
        }
    }
}

/**
 * Build the neighbours_by_surfel_frame data structure. Neighbours stay neighbours throughout and so we can compute this once
 * We assume that
 * -- neighbours by surfel frame is empty
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
        auto neighbours_of_this_surfel = surfel.neighbouring_surfels;
        for (const auto &fd : surfel.frame_data) {
            size_t frame = fd.pixel_in_frame.frame;
            vector<reference_wrapper<const Surfel>> surfels_in_this_frame = surfels_by_frame.at(frame);
            vector<reference_wrapper<const Surfel>> neighbours_in_this_frame;
            compute_intersection_of(neighbours_of_this_surfel, surfels_in_this_frame, neighbours_in_this_frame);
            neighbours_by_surfel_frame.insert(make_pair(SurfelInFrame{surfel.id, frame}, neighbours_in_this_frame));
        }
    }
}

/**
 * Compute the intersection of the two provided vectors and place the results into the third.
 */
void
Optimiser::compute_intersection_of(std::vector<size_t> neighbours_of_this_surfel,
                                   std::vector<std::reference_wrapper<const Surfel>> surfels_in_this_frame,
                                   std::vector<std::reference_wrapper<const Surfel>>& neighbours_in_this_frame) {
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