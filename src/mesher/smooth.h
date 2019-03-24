#pragma once

#include <vector>
#include "surfel.hpp"
#include <Graph/Graph.h>
#include <Graph/GraphSimplifier.h>

using animesh::Graph;
using animesh::GraphSimplifier;
using SurfelGraph    = Graph<std::size_t, void *>;
using SurfelGraphNode = Graph<std::size_t, void *>::GraphNode;
using SurfelGraphPtr = std::shared_ptr<SurfelGraph>;
using SurfelGraphSimplifier = GraphSimplifier<std::size_t, void *>;
using SurfelGraphMapping = SurfelGraphSimplifier::GraphMapping;

SurfelGraphPtr
make_surfel_graph(const std::vector<Surfel>& surfels);

/**
 * Perform orientation field optimisation.
 * Continuously step until done.
 */
void
optimise(std::vector<Surfel>& surfels,
		 const std::vector<std::vector<PointWithNormal>>& point_normals);

