//
// Created by Dave Durbin (Old) on 7/10/21.
//

#pragma once

#include "RoSyOptimiser.h"
#include <Properties/Properties.h>
#include <Surfel/SurfelGraph.h>
#include <Surfel/MultiResolutionSurfelGraph.h>

class MultiResolutionRoSyOptimiser  : public RoSyOptimiser {
public:
  MultiResolutionRoSyOptimiser(const Properties &properties, std::default_random_engine& rng );

  ~MultiResolutionRoSyOptimiser() override = default;

protected:
  void loaded_graph() override;
  void smoothing_completed(float smoothness, OptimisationResult result) override;

private:
  MultiResolutionSurfelGraph * m_multi_res_graph;
  bool m_save_interim_graphs;
  unsigned int m_num_levels;
  unsigned int m_current_level;
};
