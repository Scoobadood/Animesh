//
// Created by Dave Durbin (Old) on 25/10/21.
//

#pragma once

#include <Optimise/Optimiser.h>
#include <Properties/Properties.h>
#include <PoSy/PoSyOptimiser.h>
#include <RoSy/RoSyOptimiser.h>
#include <Surfel/MultiResolutionSurfelGraph.h>

#include <random>

class MultiResolutionOptimiser : public Optimiser {
public:
  MultiResolutionOptimiser(const Properties &properties, std::mt19937& rng );

protected:
  void loaded_graph() override;
  void smoothing_completed(float smoothness, OptimisationResult result) override;

private:
  const std::string &get_ssa_property_name() const override;
  const std::string &get_ssa_percentage_property_name() const override;

  void optimise_node(const SurfelGraphNodePtr &node) override;
  bool compare_worst_first(const SurfelGraphNodePtr &l, const SurfelGraphNodePtr &r) const override;
  float compute_node_smoothness_for_frame(
      const SurfelGraphNodePtr &node_ptr,
      size_t frame_index,
      unsigned int &num_neighbours) const override;
  void store_mean_smoothness(SurfelGraphNodePtr node, float smoothness) const override;

  MultiResolutionSurfelGraph * m_multi_res_graph;
  std::unique_ptr<PoSyOptimiser> m_posy_optimiser;
  std::unique_ptr<RoSyOptimiser> m_rosy_optimiser;
  unsigned int m_num_levels;
  unsigned int m_current_level;
};
