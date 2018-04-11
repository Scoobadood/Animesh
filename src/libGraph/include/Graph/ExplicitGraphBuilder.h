#pragma once

#include "GraphBuilder.h"

/*
 * ExplicitGraphBuilder builds graphs based on an explicit set of neighbours
 */
class ExplicitGraphBuilder : public GraphBuilder {
public:
	ExplicitGraphBuilder( std::map<int,std::vector<int>>& adjacency_map ) : m_adjacency_map{ adjacency_map }{}

	Graph * build_graph_for_elements( const std::vector<Element>& elements ) const;
private:
	std::map<int,std::vector<int>> 			m_adjacency_map;
};
