//
// Created by Dave Durbin (Old) on 4/5/21.
//

#pragma once

#include <Properties/Properties.h>
#include <Surfel/SurfelGraph.h>
#include <random>                       // default_random_engine

class Optimiser {
public:
    void set_data(const SurfelGraphPtr &surfel_graph);

    bool optimise_do_one_step();

protected:
    explicit Optimiser(Properties properties);

    void setup_termination_criteria(
            const std::string &termination_criteria_property,
            const std::string &relative_smoothness_property,
            const std::string &absolute_smoothness_property,
            const std::string &max_iterations_property);

    virtual void trace_smoothing(const SurfelGraphPtr &graph) const {};

    virtual const std::string &get_ssa_property_name() const = 0;
    virtual const std::string &get_ssa_percentage_property_name() const = 0;

    Properties m_properties;

    SurfelGraphPtr m_surfel_graph;

    // Randomise the order that neighbours are processed.
    bool m_randomise_neighour_order;

    std::default_random_engine m_random_engine;


private:
    // Termination criteria
    static const unsigned short TC_ABSOLUTE = 1 << 0;
    static const unsigned short TC_RELATIVE = 1 << 1;
    static const unsigned short TC_FIXED = 1 << 2;
    unsigned short m_termination_criteria;
    float m_term_crit_absolute_smoothness;
    float m_term_crit_relative_smoothness;
    unsigned int m_term_crit_max_iterations;

    // Optimisation
    void optimise_begin();

    void optimise_end();

    virtual void optimise_node(const SurfelGraphNodePtr &node) = 0;

    std::vector<SurfelGraphNodePtr> ssa_select_all_in_random_order();

    std::vector<SurfelGraphNodePtr> ssa_select_worst_100() const;

    std::vector<SurfelGraphNodePtr> ssa_select_worst_percentage() const;

    std::vector<SurfelGraphNodePtr> select_nodes_to_optimise();

    unsigned int m_ssa_percentage;

    void setup_ssa();

    std::function<std::vector<SurfelGraphNodePtr>(const Optimiser &)> m_node_selection_function;

    virtual bool compare_worst_first(const SurfelGraphNodePtr &l, const SurfelGraphNodePtr &r) const = 0;

    virtual float compute_node_smoothness_for_frame(
            const SurfelGraphNodePtr &node_ptr,
            size_t frame_index,
            unsigned int &num_neighbours) const = 0;

    float compute_mean_node_smoothness(const SurfelGraphNodePtr &node_ptr) const;

    float compute_mean_smoothness() const;

    enum OptimisationResult {
        NOT_COMPLETE,
        CONVERGED,
        CANCELLED,
    };

    enum OptimisationState {
        UNINITIALISED,
        INITIALISED,
        OPTIMISING,
        ENDING_OPTIMISATION
    };

    OptimisationResult m_result;
    unsigned int m_num_iterations;
    unsigned int m_num_frames;

    // Error and convergence
    float m_last_smoothness;

    OptimisationState m_state;

    unsigned short read_termination_criteria(const std::string &termination_criteria);

    // Checking for termination
    static bool user_canceled_optimise();

    static bool check_cancellation(OptimisationResult &result);

    bool maybe_check_convergence(float &latest_smoothness, OptimisationResult &result) const;

    bool maybe_check_iterations(OptimisationResult &result) const;

    bool check_termination_criteria(float &smoothness, OptimisationResult &result) const;

    static unsigned int count_number_of_frames(const SurfelGraphPtr &surfel_graph);
};