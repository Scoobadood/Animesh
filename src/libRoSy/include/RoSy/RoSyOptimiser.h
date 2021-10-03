#pragma once

#include <Optimise/Optimiser.h>
#include <Properties/Properties.h>
#include <Surfel/SurfelGraph.h>

class RoSyOptimiser : public Optimiser {
public:
  explicit RoSyOptimiser(const Properties &properties);

  virtual ~RoSyOptimiser() = default;

private:
  bool compare_worst_first(const SurfelGraphNodePtr &l, const SurfelGraphNodePtr &r) const override;

  float compute_node_smoothness_for_frame(
      const SurfelGraphNodePtr &this_node,
      size_t frame_index,
      unsigned int &num_neighbours,
      bool is_first_run) const override;

  const std::string &get_ssa_property_name() const override {
    static const std::string SSA_PROPERTY_NAME = "rosy-surfel-selection-algorithm";
    return SSA_PROPERTY_NAME;
  }

  const std::string &get_ssa_percentage_property_name() const override {
    static const std::string SSA_PERCENTAGE_PROPERTY_NAME = "rosy-ssa-percentage";
    return SSA_PERCENTAGE_PROPERTY_NAME;
  }

  void optimise_node(const SurfelGraphNodePtr &this_node) override;
  void store_mean_smoothness(SurfelGraphNodePtr node, float smoothness) const override;
  void optimise_node_with_voting(const SurfelGraphNodePtr &this_node);
  void adjust_weights_based_on_error(const std::shared_ptr<Surfel> &s1,
                                     const std::shared_ptr<Surfel> &s2,
                                     float &w_ij,
                                     float &w_ji) const;
  static std::vector<unsigned int>
  get_common_frames(const std::shared_ptr<Surfel> &s1, const std::shared_ptr<Surfel> &s2) ;

  float m_damping_factor;
  bool m_weight_for_error;
  int m_weight_for_error_steps;
};