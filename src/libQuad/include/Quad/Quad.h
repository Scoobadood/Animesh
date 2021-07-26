//
// Created by Dave Durbin (Old) on 12/7/21.
//

#pragma once

#include <Surfel/SurfelGraph.h>
#include <Eigen/Core>

typedef enum {
    EDGE_TYPE_RED = 1,
    EDGE_TYPE_BLU
} EdgeType;

using QuadGraph = animesh::Graph<Eigen::Vector3f, EdgeType>;
using QuadGraphPtr = std::shared_ptr<animesh::Graph<Eigen::Vector3f, EdgeType>>;
using QuadGraphNodePtr = std::shared_ptr<animesh::Graph<Eigen::Vector3f, EdgeType>::GraphNode>;

QuadGraphPtr
build_edge_graph(
        int frame_index,
        const SurfelGraphPtr &graph,
        float rho
);

void
collapse(int frame_index,
         QuadGraphPtr graph,
         float rho
);
