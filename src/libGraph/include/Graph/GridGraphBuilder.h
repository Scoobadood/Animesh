#pragma once

#include "GraphBuilder.h"

/*
 * GridGraphBuilder builds graphs based on a grid layout. Elements which have a 
 * difference in X, Y or Z coordinate of a fixed amount (the grid spacing) are 
 * regarded as neighbours and an edge is constructed.
 */
class GridGraphBuilder : public GraphBuilder {
public:
	GridGraphBuilder( float grid_spacing ) : m_grid_spacing{ grid_spacing }{}

	Graph * build_graph_for_elements( const std::vector<Element>& elements ) const;
	
	void check_build_edges( const void * first, const void * second, bool& build_fwd_edge, float& fwd_weight, bool& build_bwd_edge, float& bwd_weight ) const;


private:
	float 			m_grid_spacing;
};
