#include <Graph/NearestNeighbourGraphBuilder.h>
#include <algorithm>

Graph * NearestNeighbourGraphBuilder::build_graph_for_elements( const std::vector<Element>& elements ) const {
	using namespace std;
	using namespace Eigen;

	Graph * graph = new Graph( );

	for( auto& element : elements ) {
		Vector3f tan{0.0f, 0.0f, 0.0f};
		FieldElement * fe = new FieldElement( element.location(), element.normal(), tan );
		GraphNode * new_node = graph->add_node( fe );

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

void NearestNeighbourGraphBuilder::insert_edge_if_appropriate( Graph * graph, GraphNode * from, GraphNode * to ) const {
	FieldElement * fe_from = (FieldElement *) from->m_data;
	FieldElement * fe_to   = (FieldElement *) to->m_data;

	// Always add if not enough neighbours
	if( from->m_edges.size() < m_max_neighbours ) {
		graph->add_edge( fe_from, fe_to, 1.0f, nullptr);
		return;
	}


	// Check range
	GraphNode * furthest_node = std::get<2>( from->m_edges.back() );
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




bool order( const GraphNode::Edge edge1, const GraphNode::Edge edge2 ) {
	return false;
//	
}

void NearestNeighbourGraphBuilder::sort_edges( FieldElement * fe_from, std::vector<GraphNode::Edge>& edges ) const {

	std::sort( std::begin(edges), std::end(edges), [this, &fe_from](const GraphNode::Edge& edge1, const GraphNode::Edge& edge2 ){
		FieldElement * fe1 = (FieldElement *) ((GraphNode *) std::get<2>( edge1 ) )->m_data;
		FieldElement * fe2 = (FieldElement *) ((GraphNode *) std::get<2>( edge2 ) )->m_data;
		return distance_between( fe_from, fe1 ) < distance_between( fe_from, fe2 );
	});
}
