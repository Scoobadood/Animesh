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

std::shared_ptr<animesh::Graph<Eigen::Vector3f, EdgeType>>
build_edge_graph(
        int frame_index,
        const SurfelGraphPtr &graph,
        float rho
);