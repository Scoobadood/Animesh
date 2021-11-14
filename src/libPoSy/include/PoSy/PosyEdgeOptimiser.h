//
// Created by Dave Durbin (Old) on 8/11/21.
//

#pragma once

#include <Optimise/EdgeOptimiser.h>

class PosyEdgeOptimiser : public EdgeOptimiser {
public:
  PosyEdgeOptimiser(Properties properties, std::default_random_engine &rng);

protected:

  void optimise_edge(const SurfelGraph::Edge &edge) override;

private:
  float m_rho;

  float
  compute_node_smoothness_for_frame(const SurfelGraphNodePtr &node_ptr,
                                    size_t frame_index,
                                    unsigned int &num_neighbours) const override;

  void store_mean_smoothness(SurfelGraphNodePtr node, float smoothness) const override;
};

