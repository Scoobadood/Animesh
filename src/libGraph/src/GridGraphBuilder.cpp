#include <Graph/GridGraphBuilder.h>

const float EPSILON = 1e-6;

Graph * GridGraphBuilder::build_graph_for_elements( const std::vector<Element>& elements ) const {
	Graph * graph = new Graph( );

	for( auto& element : elements ) {
		Eigen::Vector3f tan{0.0f, 0.0f, 0.0f};
		FieldElement * fe = new FieldElement( element.location(), element.normal(), tan );
		graph->add_node( fe );

		// For all existing nodes, check if there should be an edge
		for( auto& pair : graph->m_data_to_node_map ) {
			if( pair.first != fe ) {
				// Check edge building
				bool fwd_edge = false;
				bool bwd_edge = false;
				float fwd_weight = 1.0f;
				float bwd_weight = 1.0f;


				check_build_edges( fe, pair.first, fwd_edge, fwd_weight, bwd_edge, bwd_weight );
				if( fwd_edge )
					graph->add_edge( fe, pair.first, fwd_weight, nullptr );

				if( bwd_edge )
					graph->add_edge( pair.first, fe, bwd_weight, nullptr );
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
void GridGraphBuilder::check_build_edges( const void * first, const void * second, bool& build_fwd_edge, float& fwd_weight, bool& build_bwd_edge, float& bwd_weight ) const {
	using namespace Eigen;

	FieldElement * fe1 = (FieldElement *) first;
	FieldElement * fe2 = (FieldElement *) second;

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