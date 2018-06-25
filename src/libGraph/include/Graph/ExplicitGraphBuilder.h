#pragma once

#include "GraphBuilder.h"

/*
 * ExplicitGraphBuilder builds graphs based on an explicit set of neighbours
 */
template <class EdgeData>
class ExplicitGraphBuilder : public GraphBuilder<EdgeData> {
public:
	ExplicitGraphBuilder( std::map<int, std::vector<int>>& adjacency_map ) : m_adjacency_map{ adjacency_map }{}

	animesh::Graph<FieldElement *, EdgeData> * build_graph_for_elements( const std::vector<Element>& elements ) const;

private:
	std::map<int,std::vector<int>> 			m_adjacency_map;
};

template<class EdgeData>
animesh::Graph<FieldElement *, EdgeData> * 
ExplicitGraphBuilder<EdgeData>::build_graph_for_elements( const std::vector<Element>& elements ) const {
	using Graph = typename animesh::Graph<FieldElement *, EdgeData>;
	using GraphNode = typename animesh::Graph<FieldElement *, EdgeData>::GraphNode;

	Graph * graph = new Graph( FieldElement::mergeFieldElements, FieldElement::propagateFieldElements);

	// Add all elements to graph
	std::vector<GraphNode *> nodes;

	for( auto& element : elements ) {
		Eigen::Vector3f tan{0.0f, 0.0f, 0.0f};
		FieldElement * fe = new FieldElement( element.location(), element.normal(), tan );
		GraphNode * gn = graph->add_node( fe );
		nodes.push_back( gn );
	}

	// For all existing nodes, insert edges to adjacent nodes according to adjacency map
	int idx = 0;
	for( auto& node : nodes ) {
		std::vector<int> neighbour_indices = m_adjacency_map.find(idx)->second;

		// For each neighbour, add an edge
		for( auto& neighbour_index : neighbour_indices ) {
			GraphNode * to_node = nodes[neighbour_index];
			graph->add_edge( node, to_node, 1.0f, nullptr );
		}
	}
	return graph;
}