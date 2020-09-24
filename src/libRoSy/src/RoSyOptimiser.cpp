#include "RoSy/RoSyOptimiser.h"

#include <RoSy/RoSy.h>
#include <Properties/Properties.h>
#include <DepthMap/DepthMap.h>
#include <Surfel/Surfel.h>
#include <Surfel/Surfel_IO.h>
#include <Surfel/PixelInFrame.h>
#include <Utilities/utilities.h>
//#include "../../mesher/hierarchical_mesher_utilities.h"

#include <Eigen/Core>
#include <random>
#include <iostream>
#include <utility>
#include <sys/stat.h>
#include "spdlog/spdlog.h"
#include <Correspondence/CorrespondenceIO.h>


static const char *SSA_SELECT_ALL_IN_RANDOM_ORDER = "select-all-in-random-order";
static const char *SSA_SELECT_WORST_100 = "select-worst-100";

/**
 * Construct one with the given properties.
 * @param properties
 */
RoSyOptimiser::RoSyOptimiser(Properties properties) : m_properties{std::move(properties)} {
    m_surfels_per_step = m_properties.getIntProperty("surfels-per-step");
    assert(m_surfels_per_step > 0);

    m_convergence_threshold = m_properties.getFloatProperty("convergence-threshold");

    auto ssa = m_properties.getProperty("surfel-selection-algorithm");
    if (ssa == SSA_SELECT_ALL_IN_RANDOM_ORDER) {
        m_surfel_selection_function = &RoSyOptimiser::ssa_select_all_in_random_order;
    } else if (ssa == SSA_SELECT_WORST_100) {
        m_surfel_selection_function = &RoSyOptimiser::ssa_select_worst_100;
    } else {
        throw std::runtime_error("Unknown surfel selection algorithm " + ssa);
    }

    m_state = UNINITIALISED;
    m_last_optimising_error = 0.0;
    m_current_level_index = 0;
    m_num_frames = 0;
    m_optimisation_cycles = 0;
    m_result = NOT_COMPLETE;
    m_num_frames = 0;
}

/**
 * Check whether user asked for optimising to halt.
 */
bool
RoSyOptimiser::user_canceled_optimise() {
    struct stat buffer{};
    return (stat("halt", &buffer) == 0);
}


/**
 * Return a const reference to the surfels being transformed so externals can play with it
 */
std::vector<std::shared_ptr<Surfel>>
RoSyOptimiser::get_surfel_data() const {
    using namespace std;
    vector<shared_ptr<Surfel>> surfels;
    for (const auto &n : m_surfel_graph.nodes()) {
        surfels.push_back(n->data());
    }
    return surfels;
}


/**
 * Save pre-smoothed surfels to file if option is set.
 */
void
RoSyOptimiser::maybe_save_presmooth_surfels_to_file(const Properties &properties) {
    if (properties.getBooleanProperty("save-presmooth-surfels")) {
        spdlog::info("   Saving presmooth Surfels");
        save_surfel_graph_to_file(
                file_name_from_template_and_level(
                        properties.getProperty("presmooth-surfel-template"),
                        m_current_level_index),
                m_surfel_graph);
    }
}

/**
 * Save post-smoothed surfels to file if option is set.
 */
void
RoSyOptimiser::maybe_save_smoothed_surfels_to_file(const Properties &properties) {
    if (properties.getBooleanProperty("save-smoothed-surfels")) {
        spdlog::info("   Saving smoothed Surfels");
        save_surfel_graph_to_file(file_name_from_template_and_level(properties.getProperty("smoothed-surfel-template"),
                                                                    m_current_level_index),
                                  m_surfel_graph);
    }
}


std::map<PixelInFrame, Eigen::Vector3f>
compute_coordinates_by_pif(const std::vector<std::vector<PixelInFrame>> &correspondences,
                           const std::vector<DepthMap> &depth_maps_by_frame,
                           const std::vector<Camera> &cameras_by_frame) {
    using namespace std;

    std::map<PixelInFrame, Eigen::Vector3f> coordinates_by_pif;
    for (const auto &correspondence_group : correspondences) {
        for (const auto &pif : correspondence_group) {
            if (coordinates_by_pif.find(pif) == end(coordinates_by_pif)) {
                auto depth = depth_maps_by_frame.at(pif.frame).depth_at(pif.pixel.x, pif.pixel.y);
                auto coord = cameras_by_frame.at(pif.frame).to_world_coordinates(pif.pixel.x, pif.pixel.y, depth);
                coordinates_by_pif.emplace(pif, coord);
            }
        }
    }
    return coordinates_by_pif;
}

void
RoSyOptimiser::generate_surfels_for_current_level() {
    using namespace spdlog;
    using namespace std;

    info("Generating surfels for level : {:d}", m_current_level_index);
    info("   Getting correspondences");
    const auto &level_depth_maps = m_depth_map_hierarchy.at(m_current_level_index);

    std::vector<Camera> level_cameras;
    for (const auto &camera : m_cameras) {
        Camera level_camera{camera};
        level_camera.set_image_size(level_depth_maps.at(0).width(), level_depth_maps.at(0).height());
        level_cameras.push_back(level_camera);
    }
    vector<vector<PixelInFrame>> correspondences = get_correspondences(m_properties, m_current_level_index,
                                                                       level_depth_maps,
                                                                       level_cameras);

    info("   Generating pif to coordinate for depth maps");
    map<PixelInFrame, Eigen::Vector3f> coordinates_by_pif = compute_coordinates_by_pif(
            correspondences,
            level_depth_maps,
            level_cameras
    );

    info("   Generating Surfels");
    m_surfel_graph = generate_surfels(level_depth_maps,
                                      correspondences,
                                      coordinates_by_pif,
                                      m_properties);

    if (!m_previous_level_surfels.empty()) {
        initialise_tangents_from_previous_level(m_surfel_graph, m_previous_surfel_graph, m_properties);
    }
    maybe_save_presmooth_surfels_to_file(m_properties);
}

void
RoSyOptimiser::set_data(const std::vector<DepthMap> &depth_maps, const std::vector<Camera> &cameras) {
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
    m_current_level_index = m_num_levels - 1;
    generate_surfels_for_current_level();
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
    m_state = INITIALISED;
}

/**
 * Start global smoothing.
 */
void
RoSyOptimiser::optimise_begin() {
    using namespace std;

    assert(m_state == INITIALISED);

    m_optimisation_cycles = 0;
    m_current_level_index = m_num_levels - 1;
    m_state = STARTING_LEVEL;
}

/**
 * Start level smoothing.
 */
void
RoSyOptimiser::optimise_begin_level() {
    using namespace spdlog;

    assert(m_state == STARTING_LEVEL);

    if (m_current_level_index != (m_num_levels - 1)) {
        generate_surfels_for_current_level();
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
    }

    m_state = OPTIMISING;
}

/**
 * End level smoothing.
 */
void
RoSyOptimiser::optimise_end_level() {
    using namespace spdlog;

    assert(m_state == ENDING_LEVEL);

    maybe_save_smoothed_surfels_to_file(m_properties);

    if (m_current_level_index == 0) {
        m_state = ENDING_OPTIMISATION;
    } else {
        --m_current_level_index;
        // TODO(dave.d) This causes compile errors. Figure out why.
        m_previous_surfel_graph = m_surfel_graph;

        // Copy all surfels in prev level to lookup table
        Surfel::surfel_by_id.clear();
        for (auto &s : m_previous_level_surfels) {
            Surfel::surfel_by_id.emplace(s->id, s);
        }
        m_state = STARTING_LEVEL;
    }
}

/**
 * Perform post-smoothing tidy up.
 */
void
RoSyOptimiser::optimise_end() {
    assert(m_state == ENDING_OPTIMISATION);

    // TODO: Consider a final state here that can transition back to INITAILISED or make both READY
    m_state = INITIALISED;
}

/**
 * Measure the change in error. If it's below some threshold, consider this level converged.
 */
void
RoSyOptimiser::check_convergence() {
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
RoSyOptimiser::compute_surfel_error_for_frame(const std::shared_ptr<Surfel> &surfel,
                                              size_t frame_id) const {
    float total_neighbour_error = 0.0f;

    // Get all neighbours in frame
    const SurfelInFrame surfel_in_frame{surfel->id, frame_id};

    const auto &this_surfel_in_this_frame = m_norm_tan_by_surfel_frame.at(surfel_in_frame);

    const auto &bounds = m_neighbours_by_surfel_frame.equal_range(surfel_in_frame);
    int num_neighbours = 0;
    int s_k = 0;
    int n_k = 0;
    for (auto np = bounds.first; np != bounds.second; ++np) {
        const auto &this_neighbour_in_this_frame = m_norm_tan_by_surfel_frame.at(
                SurfelInFrame{np->second->id, frame_id});

        // Compute the error between this surfel in this frame and the neighbour in this frame.
        total_neighbour_error += compute_error(this_surfel_in_this_frame, s_k, this_neighbour_in_this_frame, n_k);

        ++num_neighbours;
    }
    return (num_neighbours == 0)
           ? 0.0f
           : (total_neighbour_error / (float) num_neighbours);
}


/*
 * total_frame_error = 0
 * for each frame
 *   total_frame_error += compute_surfel_error_in_frame
 * next
 * total_surfel_error += (total_frame_error / num_frames)
 */
float
RoSyOptimiser::compute_surfel_error(const std::shared_ptr<Surfel> &surfel) const {
    float total_frame_error = 0.0f;

    // For each frame in which this surfel appears
    for (const auto &frame_data : surfel->frame_data) {
        // Compute the error in this frame
        total_frame_error += compute_surfel_error_for_frame(surfel, frame_data.pixel_in_frame.frame);
    }
    // Return mean error per frame
    (std::const_pointer_cast<Surfel>(surfel))->error = total_frame_error / surfel->frame_data.size();
    return surfel->error;
}

/*
    err = 0
    for each surfel
      err += surfel error
    next
    err = surfel_error / num surfels
    */
float
RoSyOptimiser::compute_mean_error_per_surfel() const {

    float total_error = 0.0f;
    for (const auto &n : m_surfel_graph.nodes()) {
        total_error += compute_surfel_error(n->data());
    }
    return total_error / m_surfel_graph.num_nodes();
}


// TODO: It feels like this is not a great error measure and we should rotate the two tangents into the same
// plane before computing the angle between them.
//
/**
 * Compute the error between two tangent vectors as the square of the angle between their 4RoSy rotations.
 *
 * @param first First normal/tangent pair.
 * @param first_k RoSy coefficient for first vector.
 * @param second Second normal/tangent pair.
 * @param second_k RoSy coefficient for second vector.
 * @return
 */
float
RoSyOptimiser::compute_error(const NormalTangent &first, int &first_k,
                             const NormalTangent &second, int &second_k) const {
    using namespace std;
    using namespace Eigen;

    // parameter order in RoSy is tangent, normal
    auto best_pair = best_rosy_vector_pair(first.tangent, first.normal, first_k,
                                           second.tangent, second.normal, second_k);
    float theta = degrees_angle_between_vectors(best_pair.first, best_pair.second);
    return (theta * theta);
}

void
RoSyOptimiser::check_cancellation() {
    if (user_canceled_optimise()) {
        m_state = ENDING_LEVEL;
        m_result = CANCELLED;
    }
}

/**
 * Select all surfels in a layer and randomize the order
 */
std::vector<SurfelGraphNodePtr>
RoSyOptimiser::ssa_select_all_in_random_order() {
    using namespace std;

    vector<size_t> indices(m_surfel_graph.num_nodes());
    iota(begin(indices), end(indices), 0);
    shuffle(begin(indices), end(indices),
            default_random_engine(chrono::system_clock::now().time_since_epoch().count()));
    vector<SurfelGraphNodePtr> selected_nodes;
    selected_nodes.reserve(indices.size());
    const auto graph_nodes = m_surfel_graph.nodes();
    for (const auto i : indices) {
        selected_nodes.push_back(graph_nodes.at(i));
    }
    return selected_nodes;
}

/**
 * Surfel selection model 2: Select top 100 error scores
 */
std::vector<SurfelGraphNodePtr>
RoSyOptimiser::ssa_select_worst_100() {
    using namespace std;

    vector<SurfelGraphNodePtr> selected_nodes;
    for (const auto &n : m_surfel_graph.nodes()) {
        selected_nodes.push_back(n);
    }
    stable_sort(begin(selected_nodes), end(selected_nodes),
                [this](const SurfelGraphNodePtr &s1, const SurfelGraphNodePtr &s2) {
                    return s1->data()->error > s2->data()->error;
                });

    selected_nodes.resize(100);
    return selected_nodes;
}

std::vector<SurfelGraphNodePtr>
RoSyOptimiser::select_nodes_to_optimise() {
    using namespace std;

    assert(m_surfel_selection_function);
    return m_surfel_selection_function(*this);
}


/**
 * Perform a single step of optimisation.
 */
bool
RoSyOptimiser::optimise_do_one_step() {
    using namespace std;

    assert(m_state != UNINITIALISED);

    if (m_state == INITIALISED) {
        optimise_begin();
    }

    if (m_state == STARTING_LEVEL) {
        optimise_begin_level();
    }

    if (m_state == OPTIMISING) {
        auto nodes_to_optimise = select_nodes_to_optimise();
        for (const auto &node : nodes_to_optimise) {
            optimise_node(node);
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

std::vector<std::pair<FrameData, FrameData>>
find_common_frame_data(const std::vector<FrameData>& f1, const std::vector<FrameData>& f2) {
    using namespace std;

    vector<pair<FrameData, FrameData>> common_frames;
    auto it1 = begin(f1);
    auto it2 = begin(f2);
    while (it1 != end(f1) && it2 != end(f2)) {
        if (it1->pixel_in_frame.frame < it2->pixel_in_frame.frame) {
            ++it1;
        } else if (it2->pixel_in_frame.frame < it1->pixel_in_frame.frame) {
            ++it2;
        } else {
            common_frames.emplace_back(*it1, *it2);
            ++it1;
            ++it2;
        }
    }
    return common_frames;
}

/**
 * Return a vector of pairs of FrameData for each frame that these surfels have in common
 */
std::vector<std::pair<FrameData, FrameData>>
find_common_frames_for_surfels(const std::shared_ptr<Surfel> &surfel1, const std::shared_ptr<Surfel> &surfel2) {
    using namespace std;

    const auto func =[](const FrameData &f1, const FrameData &f2) { return f1.pixel_in_frame.frame < f2.pixel_in_frame.frame; };
    sort(begin(surfel1->frame_data), end(surfel1->frame_data),func);
    sort(begin(surfel2->frame_data), end(surfel2->frame_data), func);

    return find_common_frame_data(surfel1->frame_data, surfel2->frame_data);
}

/**
 * Return a vector of pairs of FrameData for each frame that these surfels have in common
 */
std::vector<std::pair<FrameData, FrameData>>
find_common_frames_for_nodes(const SurfelGraphNodePtr& node1, const SurfelGraphNodePtr& node2) {
    using namespace std;

    return find_common_frames_for_surfels(node1->data(), node2->data());
}

/**
 * Given a list of frame pairs and a pair of nodes, populate norms and tans for each frame
 */
std::vector<NormalTangent>
norm_tans_for_frames(const SurfelGraphNodePtr &node1,
                     const SurfelGraphNodePtr &node2,
                     const std::vector<std::pair<FrameData, FrameData>> &common_frames) {
    using namespace std;
    using namespace Eigen;

    vector<NormalTangent> norm_tans;

    // For each neighbour
    const auto &this_surfel_ptr = node1->data();
    const auto &that_surfel_ptr = node2->data();

    // For each common frame, get normal and tangent in surfel space
    for (auto const &frame_pair : common_frames) {
        const auto& surfel_to_frame = frame_pair.first.transform;
        const auto frame_to_surfel = surfel_to_frame.transpose();
        const auto& neighbour_to_frame = frame_pair.second.transform;
        const auto& neighbour_normal_in_frame = frame_pair.second.normal;

        // Push the neighbour normal and tangent into the right frame
        // Transform the frame tangent back to the surfel space using inv. surfel matrix
        // So we need:
        //    transform from free space to frame space for tangent (stored) (we already have normal in frame space)
        //	  transform from frame space to free space using surfel data. (inv of stored)
        auto neighbour_to_surfel = frame_to_surfel * neighbour_to_frame;
        auto neighbour_tan_in_surfel_space = neighbour_to_surfel * that_surfel_ptr->tangent;
        auto neighbour_norm_in_surfel_space = frame_to_surfel * neighbour_normal_in_frame;

        norm_tans.emplace_back(neighbour_norm_in_surfel_space, neighbour_tan_in_surfel_space);
    }
    return norm_tans;
}

void
RoSyOptimiser::optimise_node(const SurfelGraphNodePtr &node) {
    using namespace Eigen;

//    3. For each neighbour (in graph)
//    4.     Find common frames
//    5.     Compute norm tans
//    6.     Smooth (returning k values)
//    7.     Update edge with k values.

    const auto &this_surfel_ptr = node->data();
    const auto & old_tangent = this_surfel_ptr->tangent;

    Vector3f new_tangent{old_tangent};
    float weight = 0;

    for( const auto& neighbour : m_surfel_graph.neighbours(node)) {
        const auto &that_surfel_ptr = neighbour->data();
        auto& edge = m_surfel_graph.edge(node, neighbour);
        float edge_weight = 1.0f;

        const auto common_frames = find_common_frames_for_nodes(node, neighbour);
        // Smooth over all frames
        for (const auto frame_pair : common_frames) {
            auto frame_idx = frame_pair.first.pixel_in_frame.frame;
            assert( frame_pair.second.pixel_in_frame.frame == frame_idx);

            int source_k;
            int target_k;

            // Compute the norm tan for this neighbour wrt this frame pair
            const auto& surfel_to_frame = frame_pair.first.transform;
            const auto frame_to_surfel = surfel_to_frame.transpose();
            const auto& neighbour_to_frame = frame_pair.second.transform;
            const auto& neighbour_normal_in_frame = frame_pair.second.normal;
            auto neighbour_to_surfel = frame_to_surfel * neighbour_to_frame;
            auto neighbour_tan_in_surfel_space = neighbour_to_surfel * that_surfel_ptr->tangent;
            auto neighbour_norm_in_surfel_space = frame_to_surfel * neighbour_normal_in_frame;

            new_tangent = average_rosy_vectors(
                    new_tangent,
                    Vector3f::UnitY(),
                    weight,
                    neighbour_tan_in_surfel_space,
                    neighbour_norm_in_surfel_space,
                    edge_weight,
                    target_k,
                    source_k);

            weight += edge_weight;

            // Store ks
            if( edge->k_ij.size() < frame_idx+1) {
                edge->k_ij.resize(frame_idx+1);
            }
            edge->k_ij.at(frame_idx) = target_k;
            if( edge->k_ji.size() < frame_idx+1) {
                edge->k_ji.resize(frame_idx+1);
            }
            edge->k_ji.at(frame_idx) = source_k;
        }
    }

    node->data()->tangent = new_tangent;
    auto vec_pair = best_rosy_vector_pair(new_tangent, Vector3f::UnitY(), old_tangent, Vector3f::UnitY());
    node->data()->last_correction = fmod(degrees_angle_between_vectors(vec_pair.first, vec_pair.second), 90.0f);
}


/**
 * Build the norm_tan_by_surfel_frame data structure for this level of the
 * surfel hierarchy. Note that tans will be updated every optimisation
 * We should calculate this once per level and then update each time we change a tan.
 */
void
RoSyOptimiser::populate_norm_tan_by_surfel_frame() {
    using namespace std;
    using namespace Eigen;

    m_norm_tan_by_surfel_frame.clear();

    /*
       For each surfel
       For each framedata in the surfel
       Generate a surfel-frame pair
       Generate a surfel-frame pair
       transform the norm and tan
       put it in the map.
     */
    for (const auto &n : m_surfel_graph.nodes()) {
        const auto &surfel = n->data();
        for (const auto &fd : surfel->frame_data) {
            SurfelInFrame surfel_in_frame{surfel->id, fd.pixel_in_frame.frame};
            Vector3f new_norm = fd.normal;
            Vector3f new_tan = fd.transform * surfel->tangent;
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
RoSyOptimiser::populate_frame_to_surfel() {
    using namespace std;
    assert(m_num_frames > 0);

    // Push an empty vector of Surfel references for each frame
    m_surfels_by_frame.clear();
    m_surfel_frame_map.clear();
    for (int frame_index = 0; frame_index < m_num_frames; ++frame_index) {
        m_surfels_by_frame.emplace_back();
    }

    // For each Surfel add it to each frame in which it appears.
    unsigned int surfel_idx = 0;
    for (const auto &n : m_surfel_graph.nodes()) {
        const auto &surfel = n->data();
        for (const auto &fd : surfel->frame_data) {
            m_surfels_by_frame.at(fd.pixel_in_frame.frame).push_back(surfel->id);
            m_surfel_frame_map.emplace(SurfelInFrame{surfel->id, fd.pixel_in_frame.frame},
                                       m_surfels_by_frame.at(fd.pixel_in_frame.frame).size() - 1);
        }
        ++surfel_idx;
    }
}

bool
RoSyOptimiser::surfel_is_in_frame(const std::string &surfel_id, size_t index) {
    return (m_surfel_frame_map.find(SurfelInFrame{surfel_id, index}) != m_surfel_frame_map.end());
}

/**
 * Given a Surfel and a frame index, return a vector of shared_ptr<Surfel> representing it's neighbours
 * in that frame.
 * @param surfel The Surfel
 * @param frame_idx The frame which we're interested in
 * @return A vector of pointers to neighbour Surfels
 */
std::vector<std::shared_ptr<Surfel>>
RoSyOptimiser::get_neighbours_of_surfel_in_frame(const std::string &surfel, unsigned int frame_idx) {
    const auto its = m_neighbours_by_surfel_frame.equal_range(SurfelInFrame{surfel, frame_idx});
    std::vector<std::shared_ptr<Surfel>> results;
    for (auto it = its.first; it != its.second; ++it) {
        results.push_back(it->second);
    }
    return results;
}

/**
 * Build the neighbours_by_surfel_frame data structure. Neighbours stay neighbours throughout and so we can compute this once
 * We assume that
 * -- surfels_by_frame is populated for this level
 *
 * But num_frames and num_surfels are both known.
 */
void
RoSyOptimiser::populate_neighbours_by_surfel_frame() {
    using namespace std;

    assert(!m_surfels_by_frame.empty());
    m_neighbours_by_surfel_frame.clear();

    // For each Surfel
    for (const auto &surfel_ptr_node : m_surfel_graph.nodes()) {
        const auto surfel_ptr = surfel_ptr_node->data();

        // Consider each neighbour
        for (const auto &neighbour : m_surfel_graph.neighbours(surfel_ptr_node)) {

            // And each frame
            for (size_t frame_idx = 0; frame_idx < m_num_frames; ++frame_idx) {
                if (!surfel_is_in_frame(surfel_ptr->id, frame_idx)) {
                    continue;
                }

                if (!surfel_is_in_frame(neighbour->data()->id, frame_idx)) {
                    continue;
                }
                SurfelInFrame sif{surfel_ptr->id, frame_idx};
                m_neighbours_by_surfel_frame.emplace(sif, neighbour->data());
            }
        }
    }
}

/**
 * For each Surfel in the current layer, find parent(s) and initialise this Surfel's
 * tangent with a combination of the parents tangents.
 * Surfels with no parents are pruned.
 * @param current_level_surfel_graph
 * @param previous_level_surfel_graph
 * @param properties
 */
void
initialise_tangents_from_previous_level(SurfelGraph &current_level_surfel_graph,
                                        const SurfelGraph &previous_level_surfel_graph,
                                        const Properties &properties) {
    using namespace std;
    vector<SurfelGraphNodePtr> orphans;

    // Construct mapping from previous graph node to this based on surfel parentage
    auto child_to_parent_surfel_id_map = compute_child_to_parent_surfel_map(
            current_level_surfel_graph,
            previous_level_surfel_graph,
            orphans);

    // Remove unparented graphnodes from this levels graph
    for (const auto &orphan : orphans) {
        current_level_surfel_graph.remove_node(orphan);
    }

    // Seed the current level surfels with tangents from their parents.
    down_propagate_tangents(child_to_parent_surfel_id_map);
}

/**
 * Create a map to allow lookup of a GraphNode from a PIF
 */
std::map<PixelInFrame, SurfelGraphNodePtr>
create_pif_to_graphnode_map(const SurfelGraph &surfel_graph) {
    using namespace std;

    map<PixelInFrame, SurfelGraphNodePtr> pif_to_graph_node;
    for (const auto &node : surfel_graph.nodes()) {
        for (const auto &frame_data : node->data()->frame_data) {
            pif_to_graph_node.emplace(frame_data.pixel_in_frame, node);
        }
    }
    return pif_to_graph_node;
}

/**
 * Given a set of parent and child GraphNodes, estalish a mapping from child to one or more parents.
 * Children with no parents are stored as IDs in unmapped
 */
std::multimap<SurfelGraphNodePtr, SurfelGraphNodePtr>
compute_child_to_parent_surfel_map(const SurfelGraph &child_graph, //
                                   const SurfelGraph &parent_graph, //
                                   std::vector<SurfelGraphNodePtr> &orphans) {
    using namespace std;

    // Construct a map from parent level PIF to parent graphnode
    const auto &pif_to_parent_graphnode_map = create_pif_to_graphnode_map(parent_graph);

    // For each PIF in each child graphnode, try to find a matching PIF in the parent graphnodemap
    // if found, these nodes are in a parent child relationship
    multimap<SurfelGraphNodePtr, SurfelGraphNodePtr> child_to_parents_graphnode_map;
    for (const auto &child_node : child_graph.nodes()) {
        size_t parents_found = 0;
        for (const auto &child_frame : child_node->data()->frame_data) {
            PixelInFrame parent_pif{child_frame.pixel_in_frame.pixel.x / 2, //
                                    child_frame.pixel_in_frame.pixel.y / 2, //
                                    child_frame.pixel_in_frame.frame};
            const auto &it = pif_to_parent_graphnode_map.find(parent_pif);
            if (it != end(pif_to_parent_graphnode_map)) {
                child_to_parents_graphnode_map.emplace(child_node, it->second);
                ++parents_found;
            } // else parent not found for this PIF
        }
        if (parents_found == 0) {
            orphans.push_back(child_node);
        }
    }
    return child_to_parents_graphnode_map;
}

// TODO(dave): Factor out the actual blending operation
/**
 * Initialise the child surfel tangents from their psarwent surfel tangents
 * where the parent-child mappings are defined in child_to_parents
 */
void
down_propagate_tangents(const std::multimap<SurfelGraphNodePtr, SurfelGraphNodePtr> &child_to_parents) {

    using namespace std;
    using namespace Eigen;
    cout << "Propagating surfel tangents from previous level" << endl;

    // for each surfel in the lower level
    auto outer_it = begin(child_to_parents);
    while (outer_it != end(child_to_parents)) {
        auto child_graphnode = outer_it->first;
        int num_parents = 0;
        Vector3f mean_tangent{0.0f, 0.0f, 0.0};
        auto inner_it = outer_it;
        while ((inner_it != end(child_to_parents)) && (inner_it->first == child_graphnode)) {
            mean_tangent += inner_it->second->data()->tangent;
            ++num_parents;
            ++inner_it;
        }
        // Set the child Surfel's tangent
        outer_it->first->data()->tangent = (mean_tangent / num_parents).normalized();
        outer_it = inner_it;
    }
}

std::vector<std::vector<PixelInFrame>>
get_correspondences(const Properties &properties,
                    unsigned int level,
                    const std::vector<DepthMap> &depth_map,
                    std::vector<Camera> &cameras) {
    using namespace std;

    vector<vector<PixelInFrame>> correspondences;

    if (properties.getBooleanProperty("load-correspondences")) {
        string corr_file_template = properties.getProperty("correspondence-file-template");
        size_t size = snprintf(nullptr, 0, corr_file_template.c_str(), level) + 1; // Extra space for '\0'
        std::unique_ptr<char[]> buf(new char[size]);
        snprintf(buf.get(), size, corr_file_template.c_str(), level);
        string file_name = std::string(buf.get(), buf.get() + size - 1); // We don't want the '\0' inside

        load_correspondences_from_file(file_name, correspondences);
    } else {
        cout << "Computing correspondences from scratch" << endl;
        throw runtime_error("Not implemented. Copy code from dm_to_pointcloud");
    }

    if (correspondences.empty()) {
        throw runtime_error("No correspondences found");
    }
    return correspondences;
}