//
// Created by Dave Durbin on 18/5/20.
//

#pragma once

#include <Optimise/Optimiser.h>
#include <Properties/Properties.h>
#include <Surfel/SurfelGraph.h>

class PoSyOptimiser : public Optimiser {
public:
    explicit PoSyOptimiser(const Properties &properties);

    virtual ~PoSyOptimiser() = default;

private:
    bool compare_worst_first(const SurfelGraphNodePtr &l, const SurfelGraphNodePtr &r) const override;

    float compute_node_smoothness_for_frame(
            const SurfelGraphNodePtr &node_ptr,
            size_t frame_index,
            unsigned int &num_neighbours) const override;

    const std::string &get_ssa_property_name() const override {
        static const std::string SSA_PROPERTY_NAME = "posy-surfel-selection-algorithm";
        return SSA_PROPERTY_NAME;
    }

    void optimise_node(const SurfelGraphNodePtr &node) override;
    void trace_smoothing(const SurfelGraphPtr &surfel_graph) const override;


    float m_rho;
};