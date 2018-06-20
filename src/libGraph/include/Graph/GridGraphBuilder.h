#pragma once

#include "GraphBuilder.h"

const float EPSILON = 1e-6;

/*
 * GridGraphBuilder builds graphs based on a grid layout. Elements which have a 
 * difference in X, Y or Z coordinate of a fixed amount (the grid spacing) are 
 * regarded as neighbours and an edge is constructed.
 */
template <class EdgeData>
class GridGraphBuilder : public GraphBuilder<EdgeData> {
	using Graph = animesh::Graph<FieldElement *, EdgeData>;
	using GraphNode = typename animesh::Graph<FieldElement *, EdgeData>::GraphNode;

public:
	/**
	 * Construct with given spacing
	 */
	GridGraphBuilder( float grid_spacing ) : m_grid_spacing{ grid_spacing }{}

	/**
	 * Build the graph
	 */
	Graph * 
	build_graph_for_elements( const std::vector<Element>& elements ) const;

	/**
	 * Determine whether two vertices should be connected by edges
	 */
	void 
	check_build_edges( GraphNode * gn1, GraphNode * gn2, 
						bool& build_fwd_edge, float& fwd_weight, 
						bool& build_bwd_edge,  float& bwd_weight ) const;

private:
	float 			m_grid_spacing;
};


template<class EdgeData>
animesh::Graph<FieldElement *, EdgeData> * 
GridGraphBuilder<EdgeData>::build_graph_for_elements( const std::vector<Element>& elements ) const {
	Graph * graph = new Graph( FieldElement::mergeFieldElements );

	for( auto& element : elements ) {
		// Make a graph node for each element
		Eigen::Vector3f tan{0.0f, 0.0f, 0.0f};
		FieldElement * fe = new FieldElement( element.location(), element.normal(), tan );
		GraphNode * gn = graph->add_node( fe );

		// For all existing nodes in graph, check if there should be an edge
		for( auto node_iter = graph->nodes().begin(); node_iter != graph->nodes().end(); ++node_iter) {
			if( (*node_iter) != gn ) {
				bool fwd_edge = false;
				bool bwd_edge = false;
				float fwd_weight = 1.0f;
				float bwd_weight = 1.0f;

				check_build_edges( gn, (*node_iter), fwd_edge, fwd_weight, bwd_edge, bwd_weight );
				if( fwd_edge )
					graph->add_edge( gn, (*node_iter), fwd_weight, nullptr );

				if( bwd_edge )
					graph->add_edge( (*node_iter), gn, bwd_weight, nullptr );
			}
		}
	}
	return graph;
}

/**
 * first The first item
 * second The second item
 * weight This will be set if the function returns true, to the weight for the edge generated	 
 * @return true if an edge should exist between the specified items.
 */
template<class EdgeData>
void GridGraphBuilder<EdgeData>::check_build_edges( GraphNode * gn1, GraphNode * gn2, 
													bool& build_fwd_edge,  float& fwd_weight, 
													bool& build_bwd_edge,  float& bwd_weight ) const {
	using namespace Eigen;
	FieldElement * fe1 = gn1->data();
	FieldElement * fe2 = gn2->data();

	Vector3f delta = fe1->m_location - fe2->m_location;

	bool add_edge = 
		( ( (fabs( fabs( delta[0] ) - m_grid_spacing)  < EPSILON ) 
		&& (fabs( delta[1] )                          < EPSILON ) 
		&& (fabs( delta[2] )                          < EPSILON ) ) ||

		 ( (fabs( delta[0] )                          < EPSILON ) 
		&& (fabs( fabs( delta[1] ) - m_grid_spacing)  < EPSILON ) 
		&& (fabs( delta[2] )                          < EPSILON ) ) ||

		 ( (fabs( delta[0] )                          < EPSILON ) 
		&& (fabs( delta[1] )                          < EPSILON ) 
		&& (fabs( fabs( delta[2] ) - m_grid_spacing)  < EPSILON ) ) );

	if( add_edge ) {
		build_fwd_edge = true;
		fwd_weight = 1.0f;
		build_bwd_edge = true;
		bwd_weight = 1.0f;
	} else {
		build_fwd_edge = false;
		build_bwd_edge = false;
	}
}