//
// Created by Dave Durbin (Old) on 7/10/21.
//

#pragma once

#include <RoSyOptimiser.h>
#include <Properties/Properties.h>
#include <Surfel/SurfelGraph.h>
#include <Surfel/MultiResolutionSurfelGraph.h>

class MultiResolutionRoSyOptimiser  : public RoSyOptimiser {
public:
  explicit MultiResolutionRoSyOptimiser(const Properties &properties);

  ~MultiResolutionRoSyOptimiser() override = default;

protected:
  void loaded_graph() override;

private:
  RoSyOptimiser * m_rosy_optimiser;
  MultiResolutionSurfelGraph * m_multi_res_graph;
  Properties m_properties;
  unsigned int m_num_levels;
  unsigned int m_current_level;
};
