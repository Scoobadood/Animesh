#pragma once

#include <Eigen/Core>
#include <Graph/Graph.h>
#include "Surfel.h"

struct SurfelGraphEdge {
    float weight;
    std::vector<std::pair<unsigned short, unsigned short>> rosy_ij;
    // Best RoSy angles
    std::vector<unsigned short> k_ij;
    std::vector<unsigned short> k_ji;
    Eigen::Vector2i posy_tij;
    Eigen::Vector2i posy_tji;
    SurfelGraphEdge( float weight ) :
            weight{weight},
            posy_tij{Eigen::Vector2i::Zero()},
            posy_tji{Eigen::Vector2i::Zero()}
    {}
};
using SurfelGraph = animesh::Graph<std::shared_ptr<Surfel>, SurfelGraphEdge>;
using SurfelGraphNodePtr = std::shared_ptr<animesh::Graph<std::shared_ptr<Surfel>, SurfelGraphEdge>::GraphNode>;
