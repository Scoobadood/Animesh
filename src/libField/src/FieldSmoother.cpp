#include <Field/Field.h>
#include <RoSy/RoSy.h>

const int MAX_ITERS_PER_TIER = 20;
const float CONVERGENCE_THRESHOLD = 0.5f;

using FieldGraph = animesh::Graph<FieldElement *, void *>;
using FieldGraphNode = typename animesh::Graph<FieldElement *, void *>::GraphNode;

/**
 * Start a brand ew smoothing session
 */
void Field::start_smoothing( ) {
	m_smoothing_tier_index = m_graph_hierarchy.size() - 1;;
	m_smoothing_current_tier = m_graph_hierarchy[m_smoothing_tier_index];
	m_smoothing_started_new_tier = true;
	m_is_smoothing = true;

	m_smoothing_last_error = calculate_error( m_smoothing_current_tier );
	if( m_tracing_enabled ) 
		std::cout << "Starting smooth. Error : " << m_smoothing_last_error << std::endl;
}

/**
 * Start a brand ew smoothing session
 */
void Field::start_smoothing_tier( ) {
	if( m_tracing_enabled ) 
		std::cout << "  Level " << m_smoothing_tier_index << std::endl;

	m_smoothing_iterations_this_tier = 0;
	m_smoothing_started_new_tier = false;
}

/**
 * Smooth the field
 */
void Field::smooth() {
	// If not smoothing, start
	if( ! m_is_smoothing ) {
		start_smoothing( );
	}

	// If we're starting a new tier...
	if( m_smoothing_started_new_tier ) {
		start_smoothing_tier( );
	}

	// Smooth the tier, possible starting a new one
	if( smooth_tier_once( m_smoothing_current_tier ) ) {
		// Converged. If there's another tier, do it
		if( m_smoothing_tier_index != 0 ) {

			m_smoothing_tier_index --;
			m_mapping_hierarchy[m_smoothing_tier_index].propagate();
			m_smoothing_current_tier = m_graph_hierarchy[m_smoothing_tier_index];
			m_smoothing_started_new_tier = true;
		}

		// Otherwise, done
		else {
			m_is_smoothing = false;
		}
	}
}

/**
 * Smooth the field
 */
void Field::smooth_completely() {
	// Debounce
	if( m_is_smoothing )
		return;

	do {
		smooth( );
	} while( m_is_smoothing );
}

/**
 * @return true if the smoothing operation has converged
 * (or has iterated enough times)
 * otherwise return false
 */
bool Field::check_convergence( float new_error ) {
	float delta = m_smoothing_last_error - new_error;
	float pct = delta / m_smoothing_last_error;
	float display_pct = std::floor( pct * 1000.0f) / 10.0f;
	m_smoothing_last_error = new_error;

	bool converged = ( display_pct >= 0.0f && display_pct < CONVERGENCE_THRESHOLD );
	if( m_tracing_enabled ) {
		std::cout << "      New Error : " << new_error << " (" << delta << ") : " << display_pct << "%" << std::endl;
	}

	if( converged ) {
		if( m_tracing_enabled ) {
			std::cout << "      Converged" << std::endl;
		}
	} else {
		if ( m_smoothing_iterations_this_tier == MAX_ITERS_PER_TIER ) {
			converged = true;
			if( m_tracing_enabled ) {
				std::cout << "      Not converging. skip to next tier" << std::endl;
			}
		}
	}
	return converged;
}

/**
 * Smooth the current tier of the hierarchy bonce and return true if it converged
 */
bool Field::smooth_tier_once ( FieldGraph * tier ) {
	using namespace Eigen;
	using namespace std;

	if( m_tracing_enabled )
		cout << "    smooth_once" << endl;

	// Extract map keys into vector and shuffle
	std::vector<FieldGraphNode *> nodes = tier->nodes();
	random_shuffle ( nodes.begin(), nodes.end() ); 

	// Iterate over permute, look up key, lookup fe and smooth
	vector<const Vector3f> new_tangents;
	for( auto node : nodes ) {
		new_tangents.push_back( calculate_smoothed_node( tier, node ) );
	}

	// Now update all of the nodes
	auto tan_iter = new_tangents.begin();
	for( auto node : nodes ) {
		node->data()->m_tangent = (*tan_iter);
		++tan_iter;
	}

	// Get the new error
	m_smoothing_iterations_this_tier ++;
	float new_error = calculate_error( tier );
	return check_convergence( new_error );
}


/**
 * Smooth the specified node
 * @return The new vector.
 */
Eigen::Vector3f Field::calculate_smoothed_node( FieldGraph * tier, FieldGraphNode * gn ) const {
	using namespace Eigen;

	FieldElement * this_fe = (FieldElement *) gn->data();
	// if( m_tracing_enabled ) 
	// 	trace_node( "smooth_node", this_fe);

	Vector3f sum = this_fe->m_tangent;
	float weight = 0;

	// For each edge from this node
	std::vector<FieldGraphNode *> neighbours = tier->neighbours( gn );
	for( auto neighbour_iter = neighbours.begin(); neighbour_iter != neighbours.end(); ++neighbour_iter ) {

		// Get the adjacent FieldElement
		FieldElement * neighbour_fe = (*neighbour_iter)->data();
		// if( m_tracing_enabled ) trace_node( "    consider neighbour", neighbour_fe );

		// Find best matching rotation
		std::pair<Vector3f, Vector3f> result = best_rosy_vector_pair( 
			sum,
			this_fe->m_normal,
			neighbour_fe->m_tangent, 
			neighbour_fe->m_normal);

		// Update the computed new tangent
		// TODO: Manage weights better
		float edge_weight = 1.0f;
		sum = (result.first * weight) + (result.second * edge_weight);
		weight += edge_weight;
		sum = reproject_to_tangent_space( sum, this_fe->m_normal );
		sum.normalize();
	}
	return sum;
}


/* ********** 
 * * Error Computations
 * *****/
/**
 * Current error in field
 */
float Field::current_error( int tier ) const {
	return calculate_error( graph_at_tier( tier ) );
}


/**
 * @return the smoothness of the entire Field
 */
float Field::calculate_error( FieldGraph * tier ) const {
	// E(O, k) :=      (oi, Rso (oji, ni, kij ))
	// For each node
	float error = 0.0f;
	for( auto node : tier->nodes() ) {
		error += calculate_error_for_node( tier, node );
	}
	return error;
}

/**
 * @return the smoothness of one node
 */
float Field::calculate_error_for_node( FieldGraph * tier, FieldGraphNode * gn ) const {
	float error = 0.0f;

	FieldElement * this_fe = (FieldElement *) gn->data();

	std::vector<FieldGraphNode *> neighbours = tier->neighbours( gn );

	for( auto n : neighbours ) {

		FieldElement * neighbour_fe = n->data();

		std::pair<Eigen::Vector3f, Eigen::Vector3f> result = best_rosy_vector_pair( 
			this_fe->m_tangent,
			this_fe->m_normal,
			neighbour_fe->m_tangent, 
			neighbour_fe->m_normal);

		float theta = angle_between_vectors( result.first, result.second );
		error += (theta*theta);
	}
	return error;
}


