//
// Created by Dave Durbin on 18/5/20.
//

#pragma once

#include <Optimise/NodeOptimiser.h>
#include <Properties/Properties.h>
#include <Surfel/SurfelGraph.h>

class PoSyOptimiser : public NodeOptimiser {
public:
  PoSyOptimiser(const Properties &properties, std::default_random_engine &rng);

  virtual ~PoSyOptimiser() = default;

protected:
  void ended_optimisation() override;

private:
  bool compare_worst_first(const SurfelGraphNodePtr &l, const SurfelGraphNodePtr &r) const override;

  float compute_smoothness_in_frame(const SurfelGraph::Edge &edge, unsigned int frame_idx) const override;

  const std::string &get_ssa_property_name() const override {
    static const std::string SSA_PROPERTY_NAME = "posy-surfel-selection-algorithm";
    return SSA_PROPERTY_NAME;
  }

  const std::string &get_ssa_percentage_property_name() const override {
    static const std::string SSA_PERCENTAGE_PROPERTY_NAME = "posy-ssa-percentage";
    return SSA_PERCENTAGE_PROPERTY_NAME;
  }

  void optimise_node(const SurfelGraphNodePtr &node) override;
  void trace_smoothing(const SurfelGraphPtr &surfel_graph) const override;
  void loaded_graph() override;

  void store_mean_smoothness(SurfelGraphNodePtr node, float smoothness) const override;

  void label_edge(SurfelGraph::Edge &edge);
  void compute_label_for_edge( //
      const SurfelGraph::Edge &edge, //
      const std::vector<unsigned int> &frames_for_edge, //
      Eigen::Vector2i &t_ij, //
      Eigen::Vector2i &t_ji) const;
  void label_edges();
  static std::vector<unsigned int>
  float m_rho;
};