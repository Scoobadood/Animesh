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
  MultiResolutionOptimiser(const Properties &properties, std::mt19937 &rng);

  void set_data(const SurfelGraphPtr &surfel_graph) override;

  bool optimise_do_one_step() override;

private:
  std::unique_ptr<RoSyOptimiser> m_rosy_optimiser;
  std::unique_ptr<PoSyOptimiser> m_posy_optimiser;
  std::unique_ptr<MultiResolutionSurfelGraph> m_multi_res_graph;
  unsigned int m_num_levels;
  unsigned int m_current_level;

  enum OptimisingState {
    UNINITIALISED,
    INITIALISED,
    OPTIMISING_ROSY,
    OPTIMISING_POSY,
    ENDING_OPTIMISATION
  };

  OptimisingState m_state;
};