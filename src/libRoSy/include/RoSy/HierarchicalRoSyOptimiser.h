//
// Created by Dave Durbin (Old) on 26/4/21.
//

#pragma once

class HierarchicalRoSyOptimiser {
    /** Cameras. One per frame */
    std::vector<Camera> m_cameras;

    /** Surfels in the previous level of smoothing if there's more than one. */
    std::vector<std::shared_ptr<Surfel>> m_previous_level_surfels;

    /** Graph of previous level surfels */
    SurfelGraphPtr m_previous_surfel_graph;

    /** Number of levels in hierarchy to create */
    size_t m_num_levels = 0;

    /** Index of the level currently being optimised */
    unsigned int m_current_level_index = 0;

    /** Depth maps by level and then frame */
    std::vector<std::vector<DepthMap>> m_depth_map_hierarchy;

    Eigen::Vector2i get_dimensions() const {
        return Eigen::Vector2i{m_depth_map_hierarchy.at(m_current_level_index).at(0).width(),
                               m_depth_map_hierarchy.at(m_current_level_index).at(0).height()
        };
    };

    unsigned int get_current_level() const {
        return m_current_level_index;
    }


};
/**
 * Do start of level set up. Mostly computing current residual error in this level.
 */
void optimise_begin_level();

/**
 * Tidy up at end of level and propagate values down to next level or else flag smoothing as converged
 * if this was level 0.
 */
void optimise_end_level();

/**
 * Generate surfels for the current optimisation level using
 * correspondences.
 */
void
generate_surfels_for_current_level();
/**
 * Create a map to allow lookup of a GraphNode from a PIF
 */
std::map<PixelInFrame, SurfelGraphNodePtr>
create_pif_to_graphnode_map(const SurfelGraphPtr & surfel_graph);

/**
 * Given a set of parent and child GraphNodes, estalish a mapping from child to one or more parents.
 * Children with no parents are stored as IDs in unmapped
 */
std::multimap<SurfelGraphNodePtr, SurfelGraphNodePtr>
compute_child_to_parent_surfel_map(const SurfelGraphPtr & child_graph, //
                                   const SurfelGraphPtr & parent_graph, //
                                   std::vector<SurfelGraphNodePtr> &orphans);

/**
 * Initialise the child surfel tangents from their psarwent surfel tangents
 * where the parent-child mappings are defined in child_to_parents
 */
void
down_propagate_tangents(const std::multimap<SurfelGraphNodePtr, SurfelGraphNodePtr> & child_to_parents);

std::vector<std::vector<PixelInFrame>>
get_correspondences(const Properties &properties,
                    unsigned int level,
                    const std::vector<DepthMap> &depth_map,
                    std::vector<Camera> &cameras);

/**
 * For each Surfel in the current layer, find parent(s) and initialise this Surfel's
 * tangent with a combination of the parents tangents.
 * Surfels with no parents are pruned.
 * @param current_level_surfel_graph
 * @param previous_level_surfel_graph
 * @param properties
 */
void
initialise_tangents_from_previous_level(SurfelGraphPtr & current_level_surfel_graph,
                                        const SurfelGraphPtr & previous_level_surfel_graph,
                                        const Properties &properties);
