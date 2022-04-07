//
// Created by Dave Durbin on 8/4/2022.
//

#pragma once

#include <memory>
#include <Surfel/MultiResolutionSurfelGraph.h>

class FieldOptimiser {
 public:
  FieldOptimiser();

  bool optimise_once();

  void set_graph(std::shared_ptr<MultiResolutionSurfelGraph> graph);

 private:
  /* After initialisation, set up ready for first pass */
  void optimise_begin();

  enum OptimisationState {
    UNINITIALISED,
    INITIALISED,
    OPTIMISING_ROSY,
  };

  std::shared_ptr<MultiResolutionSurfelGraph> m_graph;
  OptimisationState m_state;
  /* Number of optimisation passes at this level */
  int m_num_iterations;

  size_t m_current_level;

};
