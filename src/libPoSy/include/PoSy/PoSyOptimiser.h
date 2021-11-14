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

  const std::string &get_ssa_percentage_property_name() const override {
    static const std::string SSA_PERCENTAGE_PROPERTY_NAME = "posy-ssa-percentage";
    return SSA_PERCENTAGE_PROPERTY_NAME;
  }

  void optimise_node(const SurfelGraphNodePtr &node) override;
  void trace_smoothing(const SurfelGraphPtr &surfel_graph) const override;
  void loaded_graph() override;

  void store_mean_smoothness(SurfelGraphNodePtr node, float smoothness) const override;

  Eigen::Vector2f
  smooth_node_in_frame(const std::shared_ptr<Surfel> &from_surfel,
                       const Eigen::Vector2f &from_lattice_offset,
                       const std::shared_ptr<Surfel> &to_surfel,
                       const Eigen::Vector2f &to_lattice_offset,
                       unsigned int frame_idx,
                       unsigned short k_ij, float w_i,
                       unsigned short k_ji, float w_j,
                       Eigen::Vector2i & t_ij,
                       Eigen::Vector2i & t_ji ) const;

  static std::vector<unsigned int>
  filter_frames_list(const std::shared_ptr<Surfel> &from_surfel,
                     const std::shared_ptr<Surfel> &to_surfel,
                     const std::vector<unsigned int> &frames,
                     const std::string &filter_name);
  float m_rho;
};