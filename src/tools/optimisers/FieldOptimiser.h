//
// Created by Dave Durbin on 8/4/2022.
//

#pragma once

#include <memory>
#include <Surfel/MultiResolutionSurfelGraph.h>

class FieldOptimiser {
 public:
  explicit FieldOptimiser(int target_iterations, float rho);

  bool optimise_once();

  void set_graph(std::shared_ptr<MultiResolutionSurfelGraph> graph);

 private:
  /* After initialisation, set up ready for first pass */
  void optimise_begin();

  /* Smooth an individual Surfel across temporal and spatial neighbours. */
  void optimise_rosy();

  /* Smooth an individual Surfel across temporal and spatial neighbours. */
  void optimise_posy();

  void start_level();

    /* Get weights for nodes when smoothing */
  void get_weights(const std::shared_ptr<Surfel> &surfel_a,
                   const std::shared_ptr<Surfel> &surfel_b,
                   float &weight_a,
                   float &weight_b) const;

  /* Propagate mred results down wards */
  void end_level();

  /* Apply K and T values to edges */
  void label_edges();

  void label_edge(SurfelGraph::Edge &edge);

  void compute_k_for_edge( //
      const std::shared_ptr<Surfel>& from_surfel,
      const std::shared_ptr<Surfel>& to_surfel,
      unsigned int frame_idx,
      unsigned short &k_ij, //
      unsigned short &k_ji) const;

  void compute_t_for_edge( //
      const std::shared_ptr<Surfel>& from_surfel,
      const std::shared_ptr<Surfel>& to_surfel,
      unsigned int frame_idx,
      Eigen::Vector2i &t_ij, //
      Eigen::Vector2i &t_ji) const;

  enum OptimisationState {
    UNINITIALISED,
    INITIALISED,
    STARTING_NEW_LEVEL,
    OPTIMISING_ROSY,
    OPTIMISING_POSY,
    ENDING_LEVEL,
    LABEL_EDGES,
    DONE,
  };

  std::shared_ptr<MultiResolutionSurfelGraph> m_graph;
  OptimisationState m_state;
  /* Number of iterations performed in current smoothing phase */
  int m_num_iterations;

  /* Number of optimisation passes to perform */
  int m_target_iterations;

  /* The level of the mres graph being optimised */
  size_t m_current_level;

  /* Mesh spacing */
  float m_rho;
};
