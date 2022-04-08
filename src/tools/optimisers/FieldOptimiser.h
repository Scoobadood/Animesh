//
// Created by Dave Durbin on 8/4/2022.
//

#pragma once

#include <memory>
#include <Surfel/MultiResolutionSurfelGraph.h>

class FieldOptimiser {
 public:
  explicit FieldOptimiser(int target_iterations);

  bool optimise_once();

  void set_graph(std::shared_ptr<MultiResolutionSurfelGraph> graph);

 private:
  /* After initialisation, set up ready for first pass */
  void optimise_begin();

  /* Smooth an individual Surfel across temporal and spatial neighbours. */
  void optimise_rosy();

  /* Smooth an individual Surfel across temporal and spatial neighbours. */
  void optimise_posy();

    /* Get weights for nodes when smoothing */
  void get_weights(const std::shared_ptr<Surfel> &surfel_a,
                   const std::shared_ptr<Surfel> &surfel_b,
                   float &weight_a,
                   float &weight_b) const;

  enum OptimisationState {
    UNINITIALISED,
    INITIALISED,
    OPTIMISING_ROSY,
    OPTIMISING_POSY,
  };

  std::shared_ptr<MultiResolutionSurfelGraph> m_graph;
  OptimisationState m_state;
  /* Number of iterations performed in current smoothing phase */
  int m_num_iterations;

  /* Number of optimisation passes to perform */
  int m_target_iterations;

  /* The level of the mres graph being optimised */
  size_t m_current_level;
};
