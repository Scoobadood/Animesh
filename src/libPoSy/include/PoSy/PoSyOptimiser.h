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

    unsigned int m_numFrames;

    unsigned int m_optimisation_cycles;

    // Error and convergence
    float m_last_smoothness;

private:
    // Utility class to map a surfel and frame
    struct SurfelInFrame {
        std::shared_ptr<Surfel> surfel_ptr;
        size_t frame_index;

        SurfelInFrame(std::shared_ptr<Surfel> surfel_ptr, size_t f)
                : surfel_ptr{std::move(surfel_ptr)},
                  frame_index{f} {
        }

        // Sort By surfel ID
        bool operator<(const SurfelInFrame &other) const {
            if (frame_index != other.frame_index)
                return frame_index < other.frame_index;

            return surfel_ptr->id() < other.surfel_ptr->id();
        }
    };

    void check_convergence();

    float compute_total_smoothness() const;

    float compute_node_smoothness(const SurfelGraphNodePtr &node_ptr) const;

    float compute_node_smoothness_for_frame(const SurfelGraphNodePtr &node_ptr, size_t frame_index) const;

    void optimise_node(const SurfelGraphNodePtr &node);

    static unsigned int count_number_of_frames(const SurfelGraphPtr &surfel_graph);

    std::vector<SurfelGraphNodePtr> select_nodes_to_optimise() const;

    void begin_optimisation();

    void optimise_end();

    void check_cancellation();

    std::vector<SurfelGraphNodePtr> ssa_select_worst_100();

    std::function<std::vector<SurfelGraphNodePtr>(const PoSyOptimiser &)> extractSsa(const Properties &properties);

    float m_rho;

    float
    compute_smoothness(
            const Eigen::Vector3f &p_i,
            const Eigen::Vector3f &n_i,
            const Eigen::Vector3f &o_i,
            const Eigen::Vector2i &t_ij,
            const Eigen::Vector3f &p_j,
            const Eigen::Vector3f &n_j,
            const Eigen::Vector3f &o_j,
            const Eigen::Vector2i &t_ji) const;

    /**
     * State of the optimiser.
     */
    enum OptimisationState {
        UNINITIALISED,
        INITIALISED,
        OPTIMISING,
        ENDING_OPTIMISATION
    } m_state;
};