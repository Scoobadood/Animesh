//
// Created by Dave Durbin (Old) on 25/10/21.
//

#include "MultiResolutionOptimiser.h"

MultiResolutionOptimiser::MultiResolutionOptimiser(const Properties &properties, std::mt19937 &rng) //
    : Optimiser{properties, rng} //
{
  m_rosy_optimiser = std::make_unique<RoSyOptimiser>(m_properties, rng);
  m_posy_optimiser = std::make_unique<PoSyOptimiser>(m_properties, rng);
}

const std::string &
MultiResolutionOptimiser::get_ssa_property_name() const {
  spdlog::error("get_ssa_property_name is not yet implemented");
  static std::string s{"NO SSA PROPERTY NAME"};
  return s;
}

const std::string &
MultiResolutionOptimiser::get_ssa_percentage_property_name() const {
  spdlog::error("get_ssa_percentage_property_name is not yet implemented");
  static std::string s{"NO SSA PERCENTAGE PROPERTY NAME"};
  return s;
}

void
MultiResolutionOptimiser::optimise_node(const SurfelGraphNodePtr &node) {
  spdlog::error("optimise_node is not yet implemented");
}

bool
MultiResolutionOptimiser::compare_worst_first(const SurfelGraphNodePtr &l, const SurfelGraphNodePtr &r) const {
  spdlog::error("compare_worst_first is not yet implemented");
  return false;
}

float
MultiResolutionOptimiser::compute_node_smoothness_for_frame(
    const SurfelGraphNodePtr &node_ptr,
    size_t frame_index,
    unsigned int &num_neighbours) const {
  spdlog::error("compute_node_smoothness_for_frame is not yet implemented");
  return std::numeric_limits<float>::infinity();
}

void
MultiResolutionOptimiser::store_mean_smoothness(
    SurfelGraphNodePtr node,
    float smoothness) const {
  spdlog::error("store_mean_smoothness is not yet implemented");
}

void
MultiResolutionOptimiser::loaded_graph() {
  spdlog::error("loaded_graph is not yet implemented");
}

void
MultiResolutionOptimiser::smoothing_completed(float smoothness, OptimisationResult result) {
  spdlog::error("smoothing_completed is not yet implemented");
}
