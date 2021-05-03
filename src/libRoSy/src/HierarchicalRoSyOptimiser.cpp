//
// Created by Dave Durbin (Old) on 26/4/21.
//

#include "HierarchicalRoSyOptimiser.h"

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
            mean_tangent += inner_it->second->data()->tangent();
            ++num_parents;
            ++inner_it;
        }
        // Set the child Surfel's tangent
        outer_it->first->data()->setTangent((mean_tangent / num_parents).normalized());
        outer_it = inner_it;
    }
}


/**
 * Given a set of parent and child GraphNodes, estalish a mapping from child to one or more parents.
 * Children with no parents are stored as IDs in unmapped
 */
std::multimap<SurfelGraphNodePtr, SurfelGraphNodePtr>
compute_child_to_parent_surfel_map(const SurfelGraphPtr &child_graph, //
                                   const SurfelGraphPtr &parent_graph, //
                                   std::vector<SurfelGraphNodePtr> &orphans) {
    using namespace std;

    // Construct a map from parent level PIF to parent graphnode
    const auto &pif_to_parent_graphnode_map = create_pif_to_graphnode_map(parent_graph);

    // For each PIF in each child graphnode, try to find a matching PIF in the parent graphnodemap
    // if found, these nodes are in a parent child relationship
    multimap<SurfelGraphNodePtr, SurfelGraphNodePtr> child_to_parents_graphnode_map;
    for (const auto &child_node : child_graph->nodes()) {
        size_t parents_found = 0;
        for (const auto &child_frame : child_node->data()->frame_data()) {
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

/**
 * Create a map to allow lookup of a GraphNode from a PIF
 */
std::map<PixelInFrame, SurfelGraphNodePtr>
create_pif_to_graphnode_map(const SurfelGraphPtr &surfel_graph) {
    using namespace std;

    std::map<PixelInFrame, SurfelGraphNodePtr> pif_to_graph_node;
    for (const auto &node : surfel_graph->nodes()) {
        for (const auto &frame_data : node->data()->frame_data()) {
            pif_to_graph_node.emplace(frame_data.pixel_in_frame, node);
        }
    }
    return pif_to_graph_node;
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
initialise_tangents_from_previous_level(SurfelGraphPtr &current_level_surfel_graph,
                                        const SurfelGraphPtr &previous_level_surfel_graph,
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
        current_level_surfel_graph->remove_node(orphan);
    }

    // Seed the current level surfels with tangents from their parents.
    down_propagate_tangents(child_to_parent_surfel_id_map);
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
generate_surfels_for_current_level() {
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
    std::map<PixelInFrame, Eigen::Vector3f> coordinates_by_pif = compute_coordinates_by_pif(
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
set_data(const std::vector<DepthMap> &depth_maps, const std::vector<Camera> &cameras) {
    using namespace spdlog;
    using namespace std;

    assert(depth_maps.size() == cameras.size());

    m_cameras = cameras;

    // initialise depth map hierarchy
    info("Generating depth map hierarchy");
    m_depth_map_hierarchy = create_depth_map_hierarchy(m_properties, depth_maps, m_cameras);
    m_num_levels = m_depth_map_hierarchy.size();
    assert(m_num_levels > 0);
    m_current_level_index = m_num_levels - 1;
    generate_surfels_for_current_level();

    // Compute initial error values
    info("Initialising error value");
    m_last_optimising_error = compute_mean_smoothness_per_node();
    m_state = INITIALISED;
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

        // Compute initial error values
        info("Initialising error value");
        m_last_smoothness = compute_mean_smoothness_per_node();
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
        m_previous_surfel_graph = m_surfel_graph;

        // Copy all surfels in prev level to lookup table
        Surfel::m_surfel_by_id.clear();
        for (auto &s : m_previous_level_surfels) {
            Surfel::m_surfel_by_id.emplace(s->id(), s);
        }
        m_state = STARTING_LEVEL;
    }
}

/**
 * Save pre-smoothed surfels to file if option is set.
 */
void
maybe_save_presmooth_surfels_to_file(const Properties &properties) {
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




