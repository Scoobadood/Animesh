//
// Created by Dave Durbin (Old) on 8/11/21.
//

#include "EdgeOptimiser.h"

EdgeOptimiser::EdgeOptimiser(Properties properties, std::mt19937 &rng)
    : AbstractOptimiser{properties, rng}//
{}

void
EdgeOptimiser::optimise_do_pass() {
  for (const auto &edge: m_surfel_graph->edges()) {
    optimise_edge(edge);
  }

}