#include "optimise.h"

#include <RoSy/RoSy.h>
#include <Properties/Properties.h>
#include <DepthMap/DepthMap.h>
#include "utilities.h"
#include "hierarchical_mesher_utilities.h"

#include <Eigen/Core>
#include <random>
#include <iostream>
#include <utility>
#include <sys/stat.h>
#include "spdlog/spdlog.h"


static std::random_device r_device;
static std::default_random_engine r_engine(r_device());


Optimiser::Optimiser(Properties properties) : m_properties{std::move(properties)} {
    m_surfels_per_step = m_properties.getIntProperty("surfels-per-step");
    assert(m_surfels_per_step > 0);

    m_convergence_threshold = m_properties.getFloatProperty("convergence-threshold");

    m_state = UNINITIALISED;
    m_last_optimising_error = 0.0;
    m_current_level_index = 0;
    m_num_frames = 0;
    m_optimisation_cycles = 0;
    m_result = NOT_COMPLETE;
    m_num_frames = 0;
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
 * Save presmoothed surfels to file if option is set.
 */
void
Optimiser::maybe_save_presmooth_surfels_to_file(const Properties &properties) {
    if (properties.getBooleanProperty("save-presmooth-surfels")) {
        spdlog::info("   Saving presmooth Surfels");
        save_surfels_to_file(file_name_from_template_and_level(properties.getProperty("presmooth-surfel-template"),
                                                               m_current_level_index), m_current_level_surfels);
    }
}

/**
 * Save post-smoothed surfels to file if option is set.
 */
void
Optimiser::maybe_save_smoothed_surfels_to_file(const Properties &properties) {
    if (properties.getBooleanProperty("save-smoothed-surfels")) {
        spdlog::info("   Saving smoothed Surfels");
        save_surfels_to_file(file_name_from_template_and_level(properties.getProperty("smoothed-surfel-template"),
                                                               m_current_level_index), m_current_level_surfels);
    }
}


void
Optimiser::generate_surfels_for_current_level() {
    using namespace spdlog;
    using namespace std;

    m_current_level_index = m_num_levels - 1;
    info("Generating surfels for level : {:d}", m_current_level_index);
    info("   Getting correspondences");
    vector<vector<PixelInFrame>> correspondences = get_correspondences(m_properties, m_current_level_index,
                                                                       m_depth_map_hierarchy.at(m_current_level_index),
                                                                       m_cameras);

    info("   Generating Surfels");
    m_current_level_surfels = generate_surfels(m_depth_map_hierarchy.at(m_current_level_index),
                                               correspondences, m_properties);

    if (!m_previous_level_surfels.empty()) {
        initialise_tangents_from_previous_level(m_current_level_surfels, m_previous_level_surfels, m_properties);
    }
    maybe_save_presmooth_surfels_to_file(m_properties);
}

void
Optimiser::set_data(const std::vector<DepthMap> &depth_maps, const std::vector<Camera> &cameras) {
    using namespace spdlog;
    using namespace std;

    assert(depth_maps.size() == cameras.size());

    m_cameras = cameras;
    m_num_frames = cameras.size();

    // initialise depth map hierarchy
    info("Generating depth map hierarchy");
    m_depth_map_hierarchy = create_depth_map_hierarchy(m_properties, depth_maps, m_cameras);
    m_num_levels = m_depth_map_hierarchy.size();
    assert(m_num_levels > 0);
    generate_surfels_for_current_level();
    m_state = INITIALISED;
}

/**
 * Start global smoothing.
 */
void
Optimiser::optimise_begin() {
    using namespace std;

    assert(m_state == INITIALISED);

    m_optimisation_cycles = 0;
    m_state = STARTING_LEVEL;
}

/**
 * Start level smoothing.
 */
void
Optimiser::optimise_begin_level() {
    using namespace spdlog;

    assert(m_state == STARTING_LEVEL);

    if (m_current_level_index != (m_num_levels - 1)) {
        generate_surfels_for_current_level();
    }

    // First we construct a mapping from frame to surfel
    info("Mapping frames to surfels");
    populate_frame_to_surfel();

    // Then, project each Surfel's norm and tangent to each frame in which it appears
    info("Mapping surfel-frame to normal/tan");
    populate_norm_tan_by_surfel_frame();

    // Populate map of neighbours for a surfel in a frame
    info("Computing neighbours for each surfel-frame");
    populate_neighbours_by_surfel_frame();

    // Compute initial error values
    info("Initialising error value");
    m_last_optimising_error = compute_mean_error_per_surfel();

    m_state = OPTIMISING;
}

/**
 * End level smoothing.
 */
void
Optimiser::optimise_end_level() {
    using namespace spdlog;

    assert(m_state == ENDING_LEVEL);

    maybe_save_smoothed_surfels_to_file(m_properties);

    if (m_current_level_index == 0) {
        m_state = ENDING_OPTIMISATION;
    } else {
        --m_current_level_index;
        m_previous_level_surfels = m_current_level_surfels;
        Surfel::surfel_by_id.clear();
        for (auto &s : m_previous_level_surfels) {
            Surfel::surfel_by_id.emplace(s.id, std::ref(s));
        }
        m_state = STARTING_LEVEL;
    }
}

/**
 * Perform post-smoothing tidy up.
 */
void
Optimiser::optimise_end() {
    assert(m_state == ENDING_OPTIMISATION);

    // TODO: Consider a final state here that can transition back to INITAILISED or make both READY
    m_state = INITIALISED;
}

/**
 * Measure the change in error. If it's below some threshold, consider this level converged.
 */
void
Optimiser::check_convergence() {
    float latest_error = compute_mean_error_per_surfel();
    float improvement = m_last_optimising_error - latest_error;
    m_last_optimising_error = latest_error;
    float pct = (100.0f * improvement) / m_last_optimising_error;
    std::cout << "Mean error per surfel: " << latest_error << " (" << pct << "%)" << std::endl;
    if ((pct >= 0) && (std::abs(pct) < m_convergence_threshold)) {
        m_state = ENDING_LEVEL;
        m_result = CONVERGED;
    }
}


/*
 * total_neighbour_error = 0
 * for each neighbour
 *   total_neighbour_error += neighour_error
 * next
 * return total_neighbour_error / num neighours
 */
float
Optimiser::compute_surfel_error_for_frame(const std::string &surfel_id, size_t frame_id) {
    float total_neighbour_error = 0.0f;

    // Get all neighbours in frame
    const SurfelInFrame surfel_in_frame{surfel_id, frame_id};

    const auto &this_surfel_in_this_frame = m_norm_tan_by_surfel_frame.at(surfel_in_frame);
    const auto &neighbours_of_this_surfel = m_neighbours_by_surfel_frame.at(surfel_in_frame);
    for (const auto &n : neighbours_of_this_surfel) {
        const auto &this_neighbour_in_this_frame = m_norm_tan_by_surfel_frame.at(SurfelInFrame{n, frame_id});

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
Optimiser::compute_mean_error_per_surfel() {

    float total_error = 0.0f;
    for (const auto &surfel : m_current_level_surfels) {
        total_error += compute_surfel_error(surfel);
    }
    return total_error / m_current_level_surfels.size();
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
Optimiser::compute_error(const NormalTangent &first,
                         const NormalTangent &second) {
    using namespace std;
    using namespace Eigen;

    // parameter order in RoSy is tangent, normal
    pair<Vector3f, Vector3f> best_pair = best_rosy_vector_pair(first.tangent, first.normal, second.tangent,
                                                               second.normal);
    float theta = angle_between_vectors(best_pair.first, best_pair.second);
    return (theta * theta);
}

void
Optimiser::check_cancellation() {
    if (user_canceled_optimise()) {
        m_state = ENDING_LEVEL;
        m_result = CANCELLED;
    }
}

/**
 * Perform a single step of optimisation.
 */
bool
Optimiser::optimise_do_one_step() {
    using namespace std;

    assert(m_state != UNINITIALISED);

    if (m_state == INITIALISED) {
        optimise_begin();
    }

    if (m_state == STARTING_LEVEL) {
        optimise_begin_level();
    }

    if (m_state == OPTIMISING) {
        for (int i = 0; i < m_surfels_per_step; ++i) {
            // Select random surfel
            size_t surfel_idx = random_index(m_current_level_surfels.size());

            // Smooth the selected surfel frame
            optimise_surfel(surfel_idx);
        }

        // Then, project each Surfel's norm and tangent to each frame in which it appears
        populate_norm_tan_by_surfel_frame();
        ++m_optimisation_cycles;
        check_cancellation();
        check_convergence();
    }

    if (m_state == ENDING_LEVEL) {
        optimise_end_level();
    }

    if (m_state == ENDING_OPTIMISATION) {
        optimise_end();
        return true;
    }
    return false;
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
Optimiser::smooth_surfel_in_frame(size_t surfel_idx, size_t frame_idx) {
    using namespace std;
    using namespace Eigen;

    Surfel &surfel = m_current_level_surfels.at(surfel_idx);
    const FrameData &frame_data = surfel.frame_data.at(frame_idx);
    const Matrix3f &transformation_matrix = frame_data.transform;
    size_t frame_id = frame_data.pixel_in_frame.frame;

    // Get the normal and tangent of the source surfel in the specified frame
    NormalTangent source{frame_data.normal, transformation_matrix * surfel.tangent};
    SurfelInFrame sif{surfel.id, frame_id};

    const auto &neighbours_of_this_surfel = m_neighbours_by_surfel_frame.at(sif);
    vector<NormalTangent> neighbour_norm_tans;
    for (const auto &neighbour : neighbours_of_this_surfel) {
        const auto &nt = m_norm_tan_by_surfel_frame.at(SurfelInFrame{neighbour, frame_id});
        neighbour_norm_tans.push_back(nt);
    }

    Vector3f new_tangent = compute_smoothed_tangent(source, neighbour_norm_tans);
    m_current_level_surfels.at(surfel_idx).tangent = new_tangent;
}

/**
 * Return a vector of pairs of framedata for each frame that these surfels have in common
 */
std::vector<std::pair<std::reference_wrapper<const FrameData>, std::reference_wrapper<const FrameData>>>
find_common_frames_for_surfels(const Surfel &surfel1, const Surfel &surfel2) {
    using namespace std;

    vector<reference_wrapper<const FrameData>> surfel1_frames;
    vector<reference_wrapper<const FrameData>> surfel2_frames;
    for (const auto &f : surfel1.frame_data) {
        surfel1_frames.emplace_back(f);
    }
    for (const auto &f : surfel2.frame_data) {
        surfel2_frames.emplace_back(f);
    }
    sort(surfel1_frames.begin(), surfel1_frames.end(),
         [](const FrameData &f1, const FrameData &f2) { return f1.pixel_in_frame.frame < f2.pixel_in_frame.frame; });
    sort(surfel2_frames.begin(), surfel2_frames.end(),
         [](const FrameData &f1, const FrameData &f2) { return f1.pixel_in_frame.frame < f2.pixel_in_frame.frame; });

    vector<pair<reference_wrapper<const FrameData>, reference_wrapper<const FrameData>>> common_frames;
    auto it1 = surfel1.frame_data.begin();
    auto it2 = surfel2.frame_data.begin();
    while (it1 != surfel1.frame_data.end() && it2 != surfel2.frame_data.end()) {
        if (it1->pixel_in_frame.frame < it2->pixel_in_frame.frame) {
            ++it1;
        } else if (it2->pixel_in_frame.frame < it1->pixel_in_frame.frame) {
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
Optimiser::get_eligible_normals_and_tangents(std::size_t surfel_idx) const {
    using namespace std;
    using namespace Eigen;

    std::vector<NormalTangent> eligible_normals_and_tangents;

    const Surfel &this_surfel = m_current_level_surfels.at(surfel_idx);

    // For each neighbour
    int total_common_frames = 0;
    for (const auto &that_surfel_id : this_surfel.neighbouring_surfels) {
        const Surfel &that_surfel = Surfel::surfel_for_id(that_surfel_id);

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
Optimiser::optimise_surfel(size_t surfel_idx) {
    using namespace std;
    using namespace Eigen;

    Surfel &surfel = m_current_level_surfels.at(surfel_idx);

    // Get vector of eligible normal/tangent pairs
    vector<NormalTangent> neighbouring_normals_and_tangents = get_eligible_normals_and_tangents(surfel_idx);

    // Merge all neighbours; implicitly using optiminsing tier tangents
    Vector3f new_tangent = surfel.tangent;

    float weight = 0;
    for (const auto &nt : neighbouring_normals_and_tangents) {
        float edge_weight = 1.0f;
        new_tangent = average_rosy_vectors(new_tangent, Vector3f{0.0f, 1.0, 0.0f}, weight, nt.tangent, nt.normal,
                                           edge_weight);
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
Optimiser::populate_norm_tan_by_surfel_frame() {
    using namespace std;
    using namespace Eigen;

    m_norm_tan_by_surfel_frame.clear();

    /*
       For each surfel
       For each framedata in the surfel
       Generate a surfel-frame pair
       transform the norm and tan
       put it in the map.
     */
    for (const auto &surfel : m_current_level_surfels) {
        for (const auto &fd : surfel.frame_data) {
            SurfelInFrame surfel_in_frame{surfel.id, fd.pixel_in_frame.frame};
            Vector3f new_norm = fd.normal;
            Vector3f new_tan = fd.transform * surfel.tangent;
            m_norm_tan_by_surfel_frame.emplace(surfel_in_frame, NormalTangent{new_norm, new_tan});
        }
    }
}

/**
 * Build a mapping from frame to the Surfels which appear in that frame.
 * This allows us to work with frames individually.
 * surfels_by_frame is a member variable.
 */
void
Optimiser::populate_frame_to_surfel() {
    using namespace std;
    assert(m_num_frames > 0);

    // Push an empty vector of Surfel references for each frame
    m_surfels_by_frame.clear();
    for (int frame_index = 0; frame_index < m_num_frames; ++frame_index) {
        m_surfels_by_frame.emplace_back();
    }

    // For each Surfel add it to each frame in which it appears.
    for (const auto &surfel : m_current_level_surfels) {
        for (const auto &fd : surfel.frame_data) {
            m_surfels_by_frame.at(fd.pixel_in_frame.frame).push_back(surfel.id);
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
Optimiser::populate_neighbours_by_surfel_frame() {
    using namespace std;

    assert(!m_surfels_by_frame.empty());
    m_neighbours_by_surfel_frame.clear();

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
    for (const auto &surfel : m_current_level_surfels) {
        const auto &neighbours_of_this_surfel = surfel.neighbouring_surfels;
        for (const auto &fd : surfel.frame_data) {
            size_t frame = fd.pixel_in_frame.frame;

            const auto &surfels_in_this_frame = m_surfels_by_frame.at(frame);
            auto neighbours_in_this_frame = compute_intersection_of(neighbours_of_this_surfel, surfels_in_this_frame);
            m_neighbours_by_surfel_frame.emplace(SurfelInFrame{surfel.id, frame}, neighbours_in_this_frame);
        }
    }
}

/**
 * Compute the intersection of the two provided vectors and place the results into the third.
 * Assumes that the vectors are sorted
 */
std::vector<std::string>
Optimiser::compute_intersection_of(std::vector<std::string> neighbours_of_this_surfel,
                                   std::vector<std::string> surfels_in_this_frame) {
    using namespace std;

    sort(surfels_in_this_frame.begin(), surfels_in_this_frame.end());

    std::vector<std::string> neighbours_in_this_frame(
            min(surfels_in_this_frame.size(), neighbours_of_this_surfel.size()));
    auto it = set_intersection(neighbours_of_this_surfel.begin(),
                               neighbours_of_this_surfel.end(),
                               surfels_in_this_frame.begin(),
                               surfels_in_this_frame.end(),
                               neighbours_in_this_frame.begin());
    neighbours_in_this_frame.resize(it - neighbours_in_this_frame.begin());
    return neighbours_in_this_frame;
}