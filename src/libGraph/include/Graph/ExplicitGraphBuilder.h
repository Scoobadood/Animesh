#pragma once

#include "GraphBuilder.h"

/*
 * ExplicitGraphBuilder builds graphs based on an explicit set of neighbours
 */
template <class EdgeData>
class ExplicitGraphBuilder : public GraphBuilder<EdgeData> {
public:
	ExplicitGraphBuilder( std::map<int, std::vector<int>>& adjacency_map ) : m_adjacency_map{ adjacency_map }{}

	Graph<FieldElement *, EdgeData> * build_graph_for_elements( const std::vector<Element>& elements ) const;

private:
	std::map<int,std::vector<int>> 			m_adjacency_map;
};

template<class EdgeData>
Graph<FieldElement *, EdgeData> * ExplicitGraphBuilder<EdgeData>::build_graph_for_elements( const std::vector<Element>& elements ) const {
	Graph<FieldElement *, EdgeData> * graph = new Graph<FieldElement *, EdgeData>( );

	// Add all elements to graph
	std::vector<FieldElement *> field_elements;
	for( auto& element : elements ) {
		Eigen::Vector3f tan{0.0f, 0.0f, 0.0f};
		FieldElement * fe = new FieldElement( element.location(), element.normal(), tan );
		graph->add_node( fe );
		field_elements.push_back( fe );
	}

	// For all existing nodes, insert edges to adjacent nodes according to adjacency map
	int idx = 0;
	for( auto& fe : field_elements ) {
		std::vector<int> neighbour_indices = m_adjacency_map.find(idx)->second;

		// For each neighbour, add an edge
		for( auto& neighbour_index : neighbour_indices ) {
			FieldElement * to_element = field_elements[neighbour_index];
			graph->add_edge( fe, to_element, 1.0f, nullptr );
		}
	}
	return graph;
}