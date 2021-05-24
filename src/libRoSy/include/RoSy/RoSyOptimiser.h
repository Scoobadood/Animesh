#pragma once

#include <Optimise/Optimiser.h>
#include <Properties/Properties.h>
#include <Surfel/SurfelGraph.h>

class RoSyOptimiser : public Optimiser {
public:
    explicit RoSyOptimiser(const Properties& properties);

    virtual ~RoSyOptimiser() = default;

private:
    bool compare_worst_first(const SurfelGraphNodePtr &l, const SurfelGraphNodePtr &r) const override;

    float compute_node_smoothness_for_frame(
            const SurfelGraphNodePtr &node_ptr,
            size_t frame_index,
            unsigned int &num_neighbours) const override;

    const std::string &get_ssa_property_name() const override {
        static const std::string SSA_PROPERTY_NAME = "rosy-surfel-selection-algorithm";
        return SSA_PROPERTY_NAME;
    }

    const std::string &get_ssa_percentage_property_name() const override {
        static const std::string SSA_PERCENTAGE_PROPERTY_NAME = "rosy-ssa-percentage";
        return SSA_PERCENTAGE_PROPERTY_NAME;
    }

    void optimise_node(const SurfelGraphNodePtr &node) override;

    float m_damping_factor;
    bool m_weight_for_error;
    int m_weight_for_error_steps;
};