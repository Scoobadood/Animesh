#pragma once

#include "GraphBuilder.h"
#include <algorithm>

template <class EdgeData>
class NearestNeighbourGraphBuilder : public GraphBuilder<EdgeData> {
	using Graph = typename animesh::Graph<FieldElement *, EdgeData>;
	using GraphNode = typename animesh::Graph<FieldElement *, EdgeData>::GraphNode;
	using Edge = typename animesh::Graph<FieldElement *, EdgeData>::Edge;

public:
	/**
	 * Construct one
	 */
	NearestNeighbourGraphBuilder( unsigned int max_neighbours ) : m_max_neighbours{ max_neighbours }{};

	/**
	 * Build the Graph
	 */
	Graph * 
	build_graph_for_elements( const std::vector<Element>& elements ) const;

private:
	/**
	 * Insert an edge into the graph
	 */
	void insert_edge_if_appropriate( Graph * graph, GraphNode * from, GraphNode * to ) const;

	/**
	 * Sort a vector of edges
	 */
	void sort_edges( FieldElement * fe_from, std::vector<Edge>& edges ) const;

	/** The maximum number of neighbours per node */
	unsigned int m_max_neighbours;
};

/**
 * Declare function for computing distances between FEs
 */
float distance_between( const animesh::Graph<FieldElement *, void *>::GraphNode * gn1, 
						const animesh::Graph<FieldElement *, void *>::GraphNode * gn2 );


template <class EdgeData>
animesh::Graph<FieldElement *, EdgeData> * NearestNeighbourGraphBuilder<EdgeData>::build_graph_for_elements( const std::vector<Element>& elements ) const {
	using namespace std;
	using namespace Eigen;

	using Graph = typename animesh::Graph<FieldElement *, EdgeData>;
	using GraphNode = typename animesh::Graph<FieldElement *, EdgeData>::GraphNode;

	Graph * graph = new Graph( FieldElement::mergeFieldElements);

	for( auto& element : elements ) {
		Vector3f tan{0.0f, 0.0f, 0.0f};
		FieldElement * fe = new FieldElement( element.location(), element.normal(), tan );
		GraphNode * new_node = graph->add_node( fe );

		// For all existing nodes, check if there should be an edge
		for( auto node_iter : graph->nodes() ) {
			if( (node_iter) != new_node  ) {
				insert_edge_if_appropriate( graph, new_node, node_iter );
				insert_edge_if_appropriate( graph, node_iter, new_node );
			}
		}
	}

	return graph;
}

template<class EdgeData>
void NearestNeighbourGraphBuilder<EdgeData>::insert_edge_if_appropriate(
	animesh::Graph<FieldElement *, EdgeData> * graph, 
	typename animesh::Graph<FieldElement *, EdgeData>::GraphNode * from, 
	typename animesh::Graph<FieldElement *, EdgeData>::GraphNode * to ) const {

	using Graph = typename animesh::Graph<FieldElement *, EdgeData>;
	using GraphNode = typename animesh::Graph<FieldElement *, EdgeData>::GraphNode;

	std::vector<GraphNode *> neighbours = graph->neighbours( from );

	// Always add if not enough neighbours
	if( neighbours.size() < m_max_neighbours ) {
		graph->add_edge( from, to, 1.0f, nullptr);
		return;
	}

	// Otherwise check range
	// Sort neighbours based on distance
	std::sort(	std::begin(neighbours), 
				std::end(neighbours), 
				[&from](GraphNode *gn1, GraphNode *gn2){ 
					return distance_between( from, gn1 ) < distance_between( from, gn2 );
				});

	GraphNode * furthest_node = neighbours.back();

	float range_to_furthest_neighbour = distance_between(from, furthest_node);
	float range_to_new_node = distance_between(from, to);

	if( range_to_furthest_neighbour > range_to_new_node ) {
		// Remove the old furthes edge
		graph->remove_edge( from, furthest_node);
		graph->add_edge( from, to, 1.0f, nullptr);
	}
}


float distance_between( const typename animesh::Graph<FieldElement *, void*>::GraphNode * gn1, 
						const typename animesh::Graph<FieldElement *, void*>::GraphNode * gn2 ) {
	using namespace Eigen;

	return ( gn1->data()->m_location - gn2->data()->m_location ).norm();
}