//
// Created by Dave Durbin on 2019-08-11.
//

#include "surfel_hierarchy.h"
#include <sys/stat.h>
#include <random>
#include <RoSy/RoSy.h>


static const long RANDOM_SEED = 919765;


/**
 * Optimise the entire hierarchy. Do each level repeatedly until converged.
 */
void
SurfelHierarchy::optimise( ) {
    using namespace std;

    if( is_optimising ) {
        throw runtime_error( "Already optimising");
    }

    optimise_begin();
    while( optimising_should_continue() ) {
        optimise_do_one_step( );
    }
    optimise_end();
}

/**
 * Start global smoothing.
 */
void
SurfelHierarchy::optimise_begin() {
    is_optimising = true;
    smoothing_level = levels.size() - 1;
    is_starting_new_level = true;
}

/**
 * Perform post-smoothing tidy up.
 */
void
SurfelHierarchy::optimise_end() {
    is_optimising = false;
}


/**
 * Do start of level set up. Mostly computing current residual error in this level.
 */
void
SurfelHierarchy::optimise_begin_level() {
    last_smoothing_error = compute_error( );
}

/**
 * Measure the change in error. If it's below some threshold, consider this level converged.
 */
bool
SurfelHierarchy::check_convergence( ) {
    float latest_error = compute_error();
    float delta_error = fabsf( latest_error - last_smoothing_error);
    last_smoothing_error = latest_error;
    return ( delta_error / last_smoothing_error < convergence_threshold );
}

/**
 * Calculate remaining error.
 */
float
SurfelHierarchy::compute_error( ) {
    // TODO Implement me.
    return 0.0f;

}

/**
 * Check whether optimising should stop either because user asked for that to happen or else
 * because convergence has happened.
 */
bool
SurfelHierarchy::optimising_should_continue() {
    return (user_canceled_optimise() || optimising_converged);
}

/**
 * Check whether optimising should stop either because user asked for that to happen or else
 * because convergence has happened.
 */
bool
SurfelHierarchy::user_canceled_optimise() {
    struct stat buffer;
    return (stat("halt", &buffer) == 0);
}

/**
 * Perform a single step of optimisation.
 */
void
SurfelHierarchy::optimise_do_one_step() {
    using namespace std;

    if( is_starting_new_level) {
        optimise_begin_level();
    }

    optimise_do_one_surfel();

    bool converged = check_convergence( );
    if( converged ) {
        optimise_end_level();
    }
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
SurfelHierarchy::get_eligible_normals_and_tangents(std::size_t level_idx,
                                  std::size_t surfel_idx,
                                  std::vector<Eigen::Vector3f> &eligible_normals,
                                  std::vector<Eigen::Vector3f> &eligible_tangents) {
    using namespace std;
    using namespace Eigen;

    const vector<FrameData> &surfel_frames = levels.at(level_idx).at(surfel_idx).frame_data;

    // For each neighbour
    int total_common_frames = 0;
    for (size_t neighbour_idx : levels.at(level_idx).at(surfel_idx).neighbouring_surfels) {

        const vector<FrameData> &neighbour_frames = levels.at(level_idx).at(neighbour_idx).frame_data;
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
            Vector3f neighbour_tan_in_surfel_space = neighbour_to_surfel * levels.at(level_idx).at(neighbour_idx).tangent;
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
SurfelHierarchy::compute_new_tangent_for_surfel(std::size_t level_idx, std::size_t surfel_idx) {
    using namespace Eigen;
    using namespace std;

    // Get vector of eligible normal/tangent pairs
    vector<Vector3f> normals;
    vector<Vector3f> tangents;
    get_eligible_normals_and_tangents(level_idx, surfel_idx, normals, tangents);

    // Merge all neighbours; implicitly using optiminsing tier tangents
    Vector3f new_tangent = levels.at(level_idx).at(surfel_idx).tangent;

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
SurfelHierarchy::optimise_do_one_surfel() {
    using namespace std;
    using namespace Eigen;

    // Select random surfel
    size_t surfel_idx = random_index(levels.at(smoothing_level).size());

    // Update this one
    Vector3f new_tangent = compute_new_tangent_for_surfel(smoothing_level, surfel_idx);
    levels.at(smoothing_level).at(surfel_idx).tangent = new_tangent;
}

/**
 * Tidy up at end of level and propagate values down to next level or else flag smoothing as converged
 * if this was level 0.
 */
void
SurfelHierarchy::optimise_end_level( ) {
    // If curent level is 0, we have now converged
    if( smoothing_level == 0 ) {
        optimising_converged = true;
        return;
    }
    // TODO Else propagate values down and decrement level counter and set start of level flag.
}

/**
 * Select a random integer in the range [0, max_index)
 */
std::size_t
SurfelHierarchy::random_index(unsigned int max_index) {
    static std::default_random_engine e{RANDOM_SEED};
    static std::uniform_real_distribution<> dis(0, max_index);
    return std::floor(dis(e));
}


