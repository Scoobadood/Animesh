//
// Created by Dave Durbin (Old) on 12/7/21.
//

#pragma once

#include <Surfel/SurfelGraph.h>
#include <Eigen/Core>
#include <iostream>

typedef enum {
  EDGE_TYPE_NON = 0,
  EDGE_TYPE_RED,
  EDGE_TYPE_BLU
} EdgeType;

struct ConsensusGraphVertex {
  std::string surfel_id;
  Eigen::Vector3f location;

  friend std::ostream &operator<<(std::ostream &o, const ConsensusGraphVertex & v) {
    o << v.surfel_id << "("
      << v.location[0] << ", "
      << v.location[1] << ", "
      << v.location[2] << ")";
    return o;
  }

  bool operator<( const ConsensusGraphVertex& other) const {
    return surfel_id < other.surfel_id;
  }

};
using QuadGraph = animesh::Graph<ConsensusGraphVertex, EdgeType>;
using ConsensusGraphPtr = std::shared_ptr<animesh::Graph<ConsensusGraphVertex, EdgeType>>;
using ConsensusGraphNodePtr = std::shared_ptr<animesh::Graph<ConsensusGraphVertex, EdgeType>::GraphNode>;

ConsensusGraphPtr
build_consensus_graph(const SurfelGraphPtr &graph, int frame_index, float rho);

void
collapse(const ConsensusGraphPtr &graph);
