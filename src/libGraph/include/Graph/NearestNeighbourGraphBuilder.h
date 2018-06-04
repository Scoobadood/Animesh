#pragma once

#include "GraphBuilder.h"
#include <algorithm>

template <class EdgeData>
class NearestNeighbourGraphBuilder : public GraphBuilder<EdgeData> {
public:
	NearestNeighbourGraphBuilder( unsigned int max_neighbours ) : m_max_neighbours{ max_neighbours }{};

	Graph<FieldElement *, EdgeData> * build_graph_for_elements( const std::vector<Element>& elements ) const;

	void insert_edge_if_appropriate(
		Graph<FieldElement *, EdgeData> * graph, 
		GraphNode<FieldElement *, EdgeData> * from, 
		GraphNode<FieldElement *, EdgeData> * to ) const;

	void sort_edges( FieldElement * fe_from, 
		std::vector<typename GraphNode<FieldElement*, EdgeData>::Edge>& edges ) const;

	/** The maximum number of neighbours per node */
	unsigned int m_max_neighbours;
};

float distance_between( const FieldElement * fe1, const FieldElement * fe2 );

template <class EdgeData>
Graph<FieldElement *, EdgeData> * NearestNeighbourGraphBuilder<EdgeData>::build_graph_for_elements( const std::vector<Element>& elements ) const {
	using namespace std;
	using namespace Eigen;

	Graph<FieldElement *, EdgeData> * graph = new Graph<FieldElement *, EdgeData>( );

	for( auto& element : elements ) {
		Vector3f tan{0.0f, 0.0f, 0.0f};
		FieldElement * fe = new FieldElement( element.location(), element.normal(), tan );
		GraphNode<FieldElement *, EdgeData> * new_node = graph->add_node( fe );

		// For all existing nodes, check if there should be an edge
		for( auto& pair : graph->m_data_to_node_map ) {
			if( pair.first != fe ) {
				insert_edge_if_appropriate( graph, pair.second, new_node );
				insert_edge_if_appropriate( graph, new_node, pair.second );
			}
		}
	}

	return graph;
}

template<class EdgeData>
void NearestNeighbourGraphBuilder<EdgeData>::insert_edge_if_appropriate(
	Graph<FieldElement *, EdgeData> * graph, 
	GraphNode<FieldElement *, EdgeData> * from, 
	GraphNode<FieldElement *, EdgeData> * to ) const {

	FieldElement * fe_from = (FieldElement *) from->m_data;
	FieldElement * fe_to   = (FieldElement *) to->m_data;

	// Always add if not enough neighbours
	if( from->m_edges.size() < m_max_neighbours ) {
		graph->add_edge( fe_from, fe_to, 1.0f, nullptr);
		return;
	}

	// Check range
	GraphNode<FieldElement *, EdgeData> * furthest_node = std::get<2>( from->m_edges.back() );
	FieldElement * furthest_neighbour =  (FieldElement *) (furthest_node -> m_data);
	float range_to_furthest_neighbour = distance_between(fe_from, furthest_neighbour);

	float range_to_new_node = distance_between(fe_from,fe_to);

	if( range_to_furthest_neighbour > range_to_new_node ) {
		// Remove the last edge from from node
		from->m_edges.erase( from->m_edges.end() - 1 );
		graph->add_edge( fe_from, fe_to, 1.0f, nullptr);

		// Now sort the edges
		sort_edges( fe_from, from->m_edges );
	}
}


float distance_between( const FieldElement * fe1, const FieldElement * fe2 ) {
	using namespace Eigen;

	return ( fe1->m_location - fe2->m_location ).norm();
}

template <class EdgeData>
void NearestNeighbourGraphBuilder<EdgeData>::sort_edges(
	FieldElement * fe_from, 
	std::vector<typename GraphNode<FieldElement*, EdgeData>::Edge>& edges ) const {

	std::sort(
		std::begin(edges), 
		std::end(edges), 
		[this, &fe_from](const typename GraphNode<FieldElement *, EdgeData>::Edge& edge1, 
						 const typename GraphNode<FieldElement *, EdgeData>::Edge& edge2 ) {
		GraphNode<FieldElement *, EdgeData> * gn1 = std::get<2>( edge1 ); 
		FieldElement * fe1 = (FieldElement *) gn1->m_data;

		GraphNode<FieldElement *, EdgeData> * gn2 = std::get<2>( edge2 ); 
		FieldElement * fe2 = (FieldElement *) gn2->m_data;

		return distance_between( fe_from, fe1 ) < distance_between( fe_from, fe2 );
	});
}
