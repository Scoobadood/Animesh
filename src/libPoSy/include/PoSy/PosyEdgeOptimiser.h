//
// Created by Dave Durbin (Old) on 8/11/21.
//

#pragma once

#include <Optimise/EdgeOptimiser.h>

class PosyEdgeOptimiser : public EdgeOptimiser {
protected:
  PosyEdgeOptimiser(Properties properties, std::mt19937 &rng);

  void optimise_edge(const SurfelGraph::Edge &edge) override;

private:
  float m_rho;
};

