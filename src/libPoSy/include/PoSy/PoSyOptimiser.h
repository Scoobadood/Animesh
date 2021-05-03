//
// Created by Dave Durbin on 18/5/20.
//

#pragma once

#include <Properties/Properties.h>
#include <Graph/Graph.h>
#include <Surfel/Surfel.h>
#include <Surfel/SurfelGraph.h>

#include <utility>

class PoSyOptimiser {
    const unsigned short TC_ABSOLUTE = 1 << 0;
    const unsigned short TC_RELATIVE = 1 << 1;
    const unsigned short TC_FIXED = 1 << 2;

    // Termination criteria
    unsigned short m_termination_criteria;
    float m_term_crit_absolute_smoothness;
    float m_term_crit_relative_smoothness;
    unsigned int m_term_crit_max_iterations;

    /** Measure the change in error. If it's below some threshold, consider this level converged. */
    enum OptimisationResult {
        NOT_COMPLETE,
        CONVERGED,
        CANCELLED,
    };
    OptimisationResult m_result;

    void setup_termination_criteria();

    bool check_termination_criteria(float &smoothness, OptimisationResult &result) const;

public:
    explicit PoSyOptimiser(Properties properties);

    virtual ~PoSyOptimiser();

    /**
     * Perform a single step of optimisation. Return true if converged or halted.
     */
    bool optimise_do_one_step();

    /**
     * Set the optimisation data
     */
    void set_data(const SurfelGraphPtr &graph);

protected:
    Properties m_properties;

    SurfelGraphPtr m_surfel_graph;

    float m_convergence_threshold;

    std::function<std::vector<SurfelGraphNodePtr>(const PoSyOptimiser &)> m_node_selection_function;

    /**
     * Select all surfels in a layer and randomize the order
     */
    std::vector<SurfelGraphNodePtr> ssa_select_all_in_random_order();

    unsigned int m_num_frames;

    // Error and convergence
    float m_last_smoothness;

private:
    // Utility class to map a surfel and frame
    struct SurfelInFrame {
        std::shared_ptr<Surfel> surfel_ptr;
        size_t frame_index;

        // Sort By surfel ID
        bool operator<(const SurfelInFrame &other) const {
            if (frame_index != other.frame_index)
                return frame_index < other.frame_index;

            return surfel_ptr->id() < other.surfel_ptr->id();
        }
    };

    float compute_mean_node_smoothness(const SurfelGraphNodePtr &node_ptr) const;

    float compute_node_smoothness_for_frame(const SurfelGraphNodePtr &node_ptr, size_t frame_index,
                                            unsigned int &num_neighbours) const;

    void optimise_node(const SurfelGraphNodePtr &node);

    bool maybe_check_convergence(float &latest_smoothness, OptimisationResult &result) const;

    static unsigned int count_number_of_frames(const SurfelGraphPtr &surfel_graph);

    std::vector<SurfelGraphNodePtr> select_nodes_to_optimise() const;

    void optimise_begin();

    void optimise_end();

    std::vector<SurfelGraphNodePtr> ssa_select_worst_100();

    std::function<std::vector<SurfelGraphNodePtr>(const PoSyOptimiser &)> extractSsa(const Properties &properties);

    float m_rho;

    /**
     * State of the optimiser.
     */
    enum OptimisationState {
        UNINITIALISED,
        INITIALISED,
        OPTIMISING,
        ENDING_OPTIMISATION
    } m_state;

    float compute_mean_smoothness() const;
    unsigned int m_num_iterations;

    bool maybe_check_iterations(OptimisationResult &result) const;

    static bool user_canceled_optimise();

    static bool check_cancellation(OptimisationResult &result);
};