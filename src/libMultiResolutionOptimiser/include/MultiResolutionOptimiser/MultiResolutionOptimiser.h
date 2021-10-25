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
  MultiResolutionSurfelGraph * m_multi_res_graph;
  PoSyOptimiser * m_posy_optimiser;
  RoSyOptimiser * m_rosy_optimiser;
  unsigned int m_num_levels;
  unsigned int m_current_level;
};
