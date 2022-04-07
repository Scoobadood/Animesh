//
// Created by Dave Durbin on 5/4/2022.
//
#pragma once

#include "PoSyOptimiser.h"
#include <Properties/Properties.h>
#include <Surfel/SurfelGraph.h>
#include <Surfel/MultiResolutionSurfelGraph.h>

class MultiResolutionPoSyOptimiser  : public PoSyOptimiser {
 public:
  MultiResolutionPoSyOptimiser(const Properties &properties, std::default_random_engine& rng );

  ~MultiResolutionPoSyOptimiser() override = default;

 protected:
  void loaded_graph() override;
  void smoothing_completed(float smoothness, OptimisationResult result) override;

 private:
  MultiResolutionSurfelGraph * m_multi_res_graph;
  unsigned int m_num_levels;
  unsigned int m_current_level;
  bool m_save_interim_graphs;
};
