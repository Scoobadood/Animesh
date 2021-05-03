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
#include <random>

class RoSyOptimiser {
    const unsigned short TC_ABSOLUTE = 1 << 0;
    const unsigned short TC_RELATIVE = 1 << 1;
    const unsigned short TC_FIXED = 1 << 2;

    // Termination criteria
    unsigned short m_termination_criteria;
    float m_term_crit_absolute_smoothness;
    float m_term_crit_relative_smoothness;
    unsigned int m_term_crit_max_iterations;

    void setup_termination_criteria();

    /** Measure the change in error. If it's below some threshold, consider this level converged. */
    enum OptimisationResult {
        NOT_COMPLETE,
        CONVERGED,
        CANCELLED,
    };
    OptimisationResult m_result;

    bool check_termination_criteria(float &smoothness, OptimisationResult &result) const;

    static bool check_cancellation(OptimisationResult &result);

    static bool user_canceled_optimise();

    bool maybe_check_convergence(float &latest_smoothness, OptimisationResult &result) const;

    bool maybe_check_iterations(OptimisationResult &result) const;

    /** The last computed error for the given layer of the surfel network. */
    float m_last_smoothness;

    /** Number of iterations of smoothing performed */
    unsigned int m_num_iterations;

    float compute_mean_smoothness() const;

    std::vector<SurfelGraphNodePtr> node_neighbours_in_frame( const SurfelGraphNodePtr& node_ptr, unsigned int frame_index) const;

    float compute_node_smoothness_for_frame(const SurfelGraphNodePtr &surfel,
                                            size_t frame_index,
                                            unsigned int & num_neighbours
                                            ) const;

    float compute_mean_node_smoothness(const SurfelGraphNodePtr &node_ptr) const;

    /** Graph of current level surfels */
    SurfelGraphPtr m_surfel_graph;

    unsigned int m_num_frames;

    static unsigned int
    count_number_of_frames(const SurfelGraphPtr &surfel_graph);

    /** Properties to use for the optimiser */
    const Properties m_properties;

    std::default_random_engine m_random_engine;

    // Optimisation properties
    std::function<std::vector<SurfelGraphNodePtr>(RoSyOptimiser &)> m_node_selection_function;

    std::function<std::vector<SurfelGraphNodePtr>(const RoSyOptimiser &)>
    extractSsa(const Properties &properties);

    std::vector<SurfelGraphNodePtr> select_nodes_to_optimise();

    /** Surfel selection model 1: Select all in random order. The default. */
    std::vector<SurfelGraphNodePtr> ssa_select_all_in_random_order();

    /** Surfel selection model 2: Select top 100 error scores */
    std::vector<SurfelGraphNodePtr> ssa_select_worst_100();


    /** Number of Surfels to adjust each step of optimisation */
    size_t m_surfels_per_step;

    enum OptimisationState {
        UNINITIALISED,
        INITIALISED,
        STARTING_LEVEL,
        OPTIMISING,
        ENDING_LEVEL,
        ENDING_OPTIMISATION
    } m_state;

    /** Start optimisation */
    void optimise_begin();

    /** Perform post-optimisation tidy up. */
    void optimise_end();

    /** Optimise a single GraphNode */
    void optimise_node(const SurfelGraphNodePtr &node);

public:
    explicit RoSyOptimiser(Properties properties);

    /** Perform a single step of optimisation. Return true if converged or halted. */
    bool optimise_do_one_step();

    void set_data(SurfelGraphPtr &surfel_graph);

    /**
     * Return a copy of the data.
     */
    std::vector<std::shared_ptr<Surfel>> get_surfel_data() const;
};