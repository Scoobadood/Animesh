#pragma once

#include <Surfel/Surfel.h>
#include <Surfel/Surfel_Compute.h>
#include <Surfel/SurfelGraph.h>
#include <Properties/Properties.h>
#include <DepthMap/DepthMap.h>
#include <Graph/Graph.h>
#include <Camera/Camera.h>
#include "../../../mesher/types.h"
#include <Eigen/Core>
#include <map>
#include <vector>
#include <string>
#include <memory>


class RoSyOptimiser {
public:
    /**
     * Perform a single step of optimisation. Return true if converged or halted.
     */
    bool optimise_do_one_step();

    explicit RoSyOptimiser(Properties properties);

    Eigen::Vector2i get_dimensions() const {
        return Eigen::Vector2i{m_depth_map_hierarchy.at(m_current_level_index).at(0).width(),
                               m_depth_map_hierarchy.at(m_current_level_index).at(0).height()
        };
    };

    /**
     * Set the depth maps and cameras to be used by this optimiser
     * @param depth_maps
     * @param cameras
     */
    void
    set_data(const std::vector<DepthMap> &depth_maps, const std::vector<Camera> &cameras);

    /**
     * Return a copy of the data.
     */
    std::vector<std::shared_ptr<Surfel>> get_surfel_data() const;

    bool
    surfel_is_in_frame(const std::string &surfel_id, size_t index);

    /**
     * Return a (possibly empty) vector of neghbours of a surfel in a frame
     */
    std::vector<std::shared_ptr<Surfel>>
    get_neighbours_of_surfel_in_frame(const std::string &surfel, unsigned int frame_idx);

    unsigned int get_current_level() const {
        return m_current_level_index;
    }

    /**
     * Return mean error
     */
     inline float get_mean_error() const {
         return m_last_optimising_error;
     }


private:
    struct SurfelInFrame {
        std::string surfel_id;
        size_t frame_index;

        SurfelInFrame(std::string s, size_t f) : surfel_id{std::move(s)}, frame_index{f} {}

        bool operator<(const SurfelInFrame &other) const {
            if (frame_index != other.frame_index)
                return frame_index < other.frame_index;

            return surfel_id < other.surfel_id;
        }
    };

    /** Cameras. One per frame */
    std::vector<Camera> m_cameras;

    /** Surfels in the previous level of smoothing if there's more than one. */
    std::vector<std::shared_ptr<Surfel>> m_previous_level_surfels;

    /** Graph of current level surfels */
    SurfelGraph m_surfel_graph;

    /** Graph of previous level surfels */
    SurfelGraph m_previous_surfel_graph;

    /** Properties to use for the optimiser */
    const Properties m_properties;

    /** Number of levels in hierarchy to create */
    size_t m_num_levels = 0;

    /** Index of the level currently being optimised */
    unsigned int m_current_level_index = 0;

    /** Depth maps by level and then frame */
    std::vector<std::vector<DepthMap>> m_depth_map_hierarchy;

    /** Number of cycles of optimisation total */
    unsigned int m_optimisation_cycles;

    /**
     * The last computed error for the given layer of the surfel network.
     */
    float m_last_optimising_error;

    /**
     * %age change in error between iterations below which we consider convergence to have occurred
     * Expressed as value [0.0 .. 1.0]
     */
    float m_convergence_threshold;

    /**
     * Useful cache for error computation. Recalculated per level.
     * A map from a surfel/frame pair to that surfel's transformed normal and tangent in that frame.
     */
    std::map<SurfelInFrame, NormalTangent> m_norm_tan_by_surfel_frame;

    /**
     * Useful cache for error computation. Stores a list of surfels which are neighbours of the given surfels in frame
     * Key is surfel, frame
     * Value is a vector of const surfel&
     */
    std::multimap<SurfelInFrame, std::shared_ptr<Surfel>> m_neighbours_by_surfel_frame;

    /**
     * Map from frame to the surfels in it. Populated once per level.
     */
    std::vector<std::vector<std::string>> m_surfels_by_frame;

    std::map<SurfelInFrame, unsigned int> m_surfel_frame_map;

    /**
     * The number of frames in the sequence.
     */
    size_t m_num_frames;

    /**
     * Number of Surfels to adjust each step of optimisation
     */
    size_t m_surfels_per_step;

    enum OptimisationState {
        UNINITIALISED,
        INITIALISED,
        STARTING_LEVEL,
        OPTIMISING,
        ENDING_LEVEL,
        ENDING_OPTIMISATION
    } m_state;

    enum OptimisationResult {
        NOT_COMPLETE,
        CONVERGED,
        CANCELLED,
    } m_result;

    std::function<std::vector<SurfelGraphNodePtr>(RoSyOptimiser&)> m_surfel_selection_function;

    /**
     * Start optimisation
     */
    void
    optimise_begin();

    /**
     * Perform post-optimisation tidy up.
     */
    void
    optimise_end();

    /**
     * Do start of level set up. Mostly computing current residual error in this level.
     */
    void optimise_begin_level();

    /**
     * Optimise a single GraphNode
     */
    void
    optimise_node(const SurfelGraphNodePtr& node );

    /**
     * Measure the change in error. If it's below some threshold, consider this level converged.
     */
    void check_convergence();

    /**
     * Check whether the user has cancelled this optimisation
     */
    void check_cancellation();

    /**
     * Tidy up at end of level and propagate values down to next level or else flag smoothing as converged
     * if this was level 0.
     */
    void optimise_end_level();

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
    compute_error(const NormalTangent &first, int &first_k,
                  const NormalTangent &second, int &second_k) const;


    float
    compute_surfel_error_for_frame(const std::shared_ptr<Surfel> &surfel, size_t frame_id) const;

    float
    compute_surfel_error(const std::shared_ptr<Surfel> &surfel) const;

    float
    compute_mean_error_per_surfel() const;

    /**
     * Check whether optimising should stop either because user asked for that to happen or else
     * because convergence has happened.
     */
    static bool user_canceled_optimise();

    /**
     * Build the norm_tan_by_surfel_frame data structure for this level of the
     * surfel hierarchy.
     */
    void
    populate_norm_tan_by_surfel_frame();

    /**
     * Build the neighbours_by_surfel_frame data structure for this level of the
     * surfel hierarchy. Neighbours stay neighbours throughout and so we can compute this
     * once during surfel hierarchy extraction.
     */
    void
    populate_neighbours_by_surfel_frame();

    /**
     * Populate the surfels per frame map.
     */
    void
    populate_frame_to_surfel();

    /**
     * Generate surfels for the current optimisation level using
     * correspondences.
     */
    void
    generate_surfels_for_current_level();

    /**
     * Save presmoothed surfels to file if option is set.
     */
    void
    maybe_save_presmooth_surfels_to_file(const Properties &properties);

    /**
     * Save post-smoothed surfels to file if option is set.
     */
    void
    maybe_save_smoothed_surfels_to_file(const Properties &properties);

    std::vector<SurfelGraphNodePtr>
    select_nodes_to_optimise();

    /**
     * Surfel selection model 1: Select all in random order. The default.
     */
    std::vector<SurfelGraphNodePtr>
    ssa_select_all_in_random_order();

    /**
     * Surfel selection model 2: Select top 100 error scores
     */
    std::vector<SurfelGraphNodePtr>
    ssa_select_worst_100();

};

/**
 * Create a map to allow lookup of a GraphNode from a PIF
 */
std::map<PixelInFrame, SurfelGraphNodePtr>
create_pif_to_graphnode_map(const SurfelGraph & surfel_graph);

/**
 * Given a set of parent and child GraphNodes, estalish a mapping from child to one or more parents.
 * Children with no parents are stored as IDs in unmapped
 */
std::multimap<SurfelGraphNodePtr, SurfelGraphNodePtr>
compute_child_to_parent_surfel_map(const SurfelGraph & child_graph, //
                                   const SurfelGraph & parent_graph, //
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
 * Remove any surfels which cannot be found from the neighbours of remaining surfels
 */
void
prune_surfel_neighbours(std::vector<std::shared_ptr<Surfel>> &surfels,
                        std::vector<std::shared_ptr<Surfel>> &surfels_to_remove,
                        const Properties &properties);

/**
 * Remove the previous level surfels from the Surfel::map
 */
void
unmap_surfels(const std::vector<std::shared_ptr<Surfel>> &surfel_ids);

/**
 * Remove the previous level surfels from the Surfel::map
 */
void
unmap_surfels(const std::vector<std::shared_ptr<Surfel>> &surfels);

/**
 * Given a list of surfel IDs, remove them from the surfel list.
 * The properties object is consulted to check whether to log this removal or not.
 */
void
remove_surfels_by_id(std::vector<std::shared_ptr<Surfel>> &surfels,
                     std::vector<std::shared_ptr<Surfel>> &surfels_to_remove,
                     const Properties &properties);


/**
 * For each Surfel in the current layer, find parent(s) and initialise this Surfel's
 * tangent with a combination of the parents tangents.
 * Surfels with no parents are pruned.
 * @param current_level_surfel_graph
 * @param previous_level_surfel_graph
 * @param properties
 */
void
initialise_tangents_from_previous_level(SurfelGraph & current_level_surfel_graph,
                                        const SurfelGraph & previous_level_surfel_graph,
                                        const Properties &properties);
