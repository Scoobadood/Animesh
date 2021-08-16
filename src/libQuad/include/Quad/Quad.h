//
// Created by Dave Durbin (Old) on 12/7/21.
//

#pragma once

#include <Surfel/SurfelGraph.h>
#include <Eigen/Core>

typedef enum {
  EDGE_TYPE_NON = 0,
  EDGE_TYPE_RED,
  EDGE_TYPE_BLU
} EdgeType;

struct QuadGraphVertex {
  std::string surfel_id;
  Eigen::Vector3f location;
};
using QuadGraph = animesh::Graph<QuadGraphVertex, EdgeType>;
using QuadGraphPtr = std::shared_ptr<animesh::Graph<QuadGraphVertex, EdgeType>>;
using QuadGraphNodePtr = std::shared_ptr<animesh::Graph<QuadGraphVertex, EdgeType>::GraphNode>;

QuadGraphPtr
build_edge_graph(
    int frame_index,
    const SurfelGraphPtr &graph,
    float rho
);

void
collapse(int frame_index,
         const QuadGraphPtr &graph,
         float rho
);
