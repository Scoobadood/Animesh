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

  /* Smooth an individual Surfel across temporal and spatial neighbours. */
  void optimise_rosy();

  /* Get weights for nodes when smoothing */
  void get_weights(const std::shared_ptr<Surfel> &surfel_a,
                   const std::shared_ptr<Surfel> &surfel_b,
                   float &weight_a,
                   float &weight_b) const;

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
