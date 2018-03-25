#include <Field/Field.h>
#include <Element/Element.h>
#include <VectorAngle/VectorAngle.h>
#include <Eigen/Core>
#include <Eigen/Geometry>

//#define TRACE  1

#include <iostream>

const float EPSILON = 1e-6;

Field::~Field( ) {
	delete m_graph;
}


/**
 * Construct the field using a graph builder and some elements
 */
Field::Field( const GraphBuilder * const graph_builder, const std::vector<Element>& elements ) {
	m_graph = graph_builder->build_graph_for_elements( elements );

	// Initialise field tangents to random values
	for( auto node_iter = m_graph->m_data_to_node_map.begin(); node_iter != m_graph->m_data_to_node_map.end(); ++node_iter ) {
		FieldElement * fe = (FieldElement *) (*node_iter).first;


		Eigen::Vector3f random = Eigen::VectorXf::Random(3);
		random = random.cross( fe->m_normal ).normalized( );
		fe->m_tangent = random;
	}
}

/**
 * Construct a planar field centred at (0,0,0)
 * @param dim_x The number of points in the X plane
 * @param dim_y The number of points in the Y plane
 * @param grid_spacing The space between grid points
 * @param make_fixed If true, set the field tangents to the lowest energy/solved position
 */
Field * Field::planar_field( std::size_t dim_x, std::size_t dim_y, float grid_spacing, bool make_fixed ) {
	using namespace Eigen;

	std::vector<Element> elements;
	for( int y = 0; y<dim_y; y++ ) {
		for( int x = 0; x<dim_x; x++ ) {
			Vector3f location { x, y, 0.0f };
			Vector3f normal{ 0.0f, 0.0f, 1.0f };
			Element e{location, normal };
			elements.push_back( e );
		}
	}

	GridGraphBuilder * ggb = new GridGraphBuilder( grid_spacing );
	Field * field = new Field( ggb, elements );

	delete ggb;


	// Make a set of planar tangents
	if( make_fixed ) {
		std::vector< const Eigen::Vector3f > good_tangents;
		for( std::size_t i=0; i< field->size(); ++i ) {
			good_tangents.push_back( Eigen::Vector3f{ 0.0f, 1.0f, 0.0f } );
		}

		field->set_tangents( good_tangents );
	}

	return field;
}

/**
 * Construct a spherical field centred at (0,0,0)
 * @param radius The radius of the sphere to be constructed
 * @param theta_steps The number of steps around the sphere (in XZ plane)
 * @param phi_steps The number of steps in the Y direction
 * @param make_fixed If true, set the field tangents to the lowest energy/solved position
 */
Field * Field::spherical_field( float radius, std::size_t theta_steps, std::size_t phi_steps, bool make_fixed ) {
	using namespace Eigen;

	float maxTheta = 1 * M_PI;
	float maxPhi = M_PI / 2.0f;
	float deltaTheta = maxTheta / theta_steps;
	float deltaPhi   = maxPhi / phi_steps;

	std::vector<Element> elements;
	for( float theta = 0.0f; theta < maxTheta; theta +=  deltaTheta) {
		for( float phi = deltaPhi; phi<maxPhi; phi += deltaPhi ) {
			Vector3f location { radius * sin(phi) * cos(theta),   radius * cos( phi ), radius * sin(phi) * sin(theta) };
			Vector3f normal = location.normalized();

			Element e{ location, normal };
			elements.push_back( e );
		}
	}

	NearestNeighbourGraphBuilder * gb = new NearestNeighbourGraphBuilder( 8 );
	Field * field = new Field( gb, elements );
	delete gb;

	if( make_fixed ) {
		for( auto pair_iter = field->m_graph->m_data_to_node_map.begin();
			      pair_iter != field->m_graph->m_data_to_node_map.end();
			      ++pair_iter ) {

			FieldElement *fe = (FieldElement *)((*pair_iter).first);

			// Find cross product of normal and vertical
			Vector3f v{0.0f, 1.0f, 0.0f };
			if( std::fabs( fe->m_normal[1] - 1.0f ) < EPSILON ) {
				v = Vector3f{1.0f, 0.0f, 0.0f };
			}
			Vector3f hor_tan = v.cross( fe->m_normal );
			Vector3f ver_tan = hor_tan.cross(fe->m_normal);
			fe->m_tangent    = ver_tan.normalized( );
		}
	}

	return field;
}

Field * Field::triangular_field( float tri_radius ) {
	using namespace Eigen;

	Vector3f v1{ 0.0f, 0.0f, tri_radius };
	Vector3f v2{ tri_radius * -0.866f, 0.0f, tri_radius * -0.5f };
	Vector3f v3{ tri_radius *  0.866f, 0.0f, tri_radius * -0.5f };

	Vector3f vv1 = v3 - v2;
	Vector3f vv2 = v1 - v2;

	std::vector<Element> elements;
	for( int i=0; i<200; i++ ) {

		float c1 = rand() %100 / 100.0f;
		float c2 = rand() %100 / 100.0f;

		if( c1 + c2 >= 1 ) {
			c1 = 1.0f - c1;
			c2 = 1.0f - c2;
		}

		Vector3f location  = v1 + c1 * vv1 + c2 * vv2;
		Vector3f normal{ 0, 1, 0 };
		Element e{ location, normal };
		elements.push_back( e );
	}
	NearestNeighbourGraphBuilder * gb = new NearestNeighbourGraphBuilder( 8 );
	Field * field = new Field( gb, elements );

	delete gb;

	return field;
}

Field * Field::cubic_field( std::size_t cube_size, bool make_fixed) {
	using namespace Eigen;

	float minVal = 0.5f - ( cube_size / 2.0f );
	float maxVal = ( cube_size / 2.0f ) - 0.5f;

	std::vector<Element> elements;

	Vector3f normalZ{ 0, 0, 1 };
	for( float x = minVal; x <= maxVal; x +=  1.0f) {
		for( float y = minVal; y <= maxVal; y+= 1.0f ) {

			Vector3f location { x, y, minVal - 0.5f };
			Element e{ location, -normalZ };
			elements.push_back( e );

			Vector3f location2 { x, y, maxVal + 0.5f };
			Element e2{ location2, normalZ };
			elements.push_back( e2 );
		}
	}
	Vector3f normalY{ 0, 1, 0 };
	for( float x = minVal; x <= maxVal; x +=  1.0f) {
		for( float z = minVal; z <= maxVal; z+= 1.0f ) {
			Vector3f location { x, minVal - 0.5f, z };
			Element e{ location, -normalY };
			elements.push_back( e );

			Vector3f location2 { x, maxVal + 0.5f, z };
			Element e2 {location2, normalY };
			elements.push_back( e2 );
		}
	}
	Vector3f normalX{ 1, 0, 0 };
	for( float z = minVal; z <= maxVal; z +=  1.0f) {
		for( float y = minVal; y <= maxVal; y+= 1.0f ) {
			Vector3f location { minVal - 0.5f, y, z };
			Element e{ location, -normalX };
			elements.push_back( e );

			Vector3f location2 { maxVal + 0.5f, y, z };
			Element e2{location2, normalX };
			elements.push_back( e2 );
		}
	}

	NearestNeighbourGraphBuilder * gb = new NearestNeighbourGraphBuilder( 8 );
	Field * field = new Field( gb, elements );
	delete gb;


	if( make_fixed ) {
		for( auto pair_iter = field->m_graph->m_data_to_node_map.begin();
			      pair_iter != field->m_graph->m_data_to_node_map.end();
			      ++pair_iter ) {

			FieldElement *fe = (FieldElement *)((*pair_iter).first);

			if( fabsf(fe->m_tangent[0]) < EPSILON && 
				fabsf(fe->m_tangent[1]) < EPSILON && 
				fabsf(fe->m_tangent[2]) < EPSILON ) {
#ifdef TRACE
				std::cout << "location " << fe->m_location[0] << "," << fe->m_location[1] << "," << fe->m_location[2] << ")" << std::endl;
#endif
				throw std::invalid_argument( "Found zero tangent for fe");
			}

			if( fe->m_normal[0] < -EPSILON || fe->m_normal[0] > EPSILON || fe->m_normal[2] < -EPSILON || fe->m_normal[2] > EPSILON ) {
				fe->m_tangent = Vector3f{ 0.0f, 1.0f, 0.0f };
			} else if ( fe->m_normal[1] < -EPSILON || fe->m_normal[1] > EPSILON ) {
				fe->m_tangent = Vector3f{ 0.0f, 0.0f, 1.0f };
			} else {
				throw std::invalid_argument( "Unexpected normal in cubic_field");
			}
		}
	}

	return field;
}



/**
 * @return the size of the ifled
 */
std::size_t Field::size() const {
	return m_graph->m_data_to_node_map.size();
}


/**
 * Smooth the field once, applying smoothing to each node
 * @return the largest error in tangent
 */
float Field::smooth_once( ) {
#ifdef TRACE
	std::cout << "smooth_once" << std::endl;
#endif

	using namespace Eigen;

	std::vector<const Vector3f> new_tangents;

	// For each graphnode, compute the smoothed tangent
	for( auto map_iter = m_graph->m_data_to_node_map.begin(); map_iter != m_graph->m_data_to_node_map.end(); ++map_iter ) {
		GraphNode * g = (*map_iter).second;
		Vector3f v = get_smoothed_tangent_data_for_node( g );
		new_tangents.push_back( v );
	}

	// And then update the tangents
	float cost = set_tangents( new_tangents );

	return cost;
}

/**
 * Set all the tangents in the field to specific values.
 * @param new_tangents The new tangents
 * @return The difference between the old and new values
 */
float Field::set_tangents( const std::vector<const Eigen::Vector3f>& new_tangents ) {
	using namespace Eigen;

	if( new_tangents.size() != m_graph->m_data_to_node_map.size() ) 
		throw std::invalid_argument( "Must be one tangent for each node");


	float cost = 0.0f;
	auto tan_iter = new_tangents.begin();
	for( auto map_iter = m_graph->m_data_to_node_map.begin(); map_iter != m_graph->m_data_to_node_map.end(); ++map_iter, ++tan_iter ) {
		FieldElement * fe = (FieldElement *) (*map_iter).first;

		const Vector3f current_tangent = fe->m_tangent;
		const Vector3f new_tangent = (*tan_iter);
		float a = angleBetweenVectors( current_tangent, new_tangent );
		cost += (a*a);
		fe->m_tangent = new_tangent;
	}
	return cost;
}

/**
 * Smooth node
 * Smooth a node in the field by averaging it's neighbours
 * @return The new vector.
 */
Eigen::Vector3f Field::get_smoothed_tangent_data_for_node( const GraphNode * const gn ) const {
	using namespace Eigen;

	FieldElement * this_fe = (FieldElement *) gn->m_data;

	if( m_tracing_enabled ) {
		std::cout << "get_smoothed_tangent_data_for_node( l=( " 
			<< this_fe->m_location[0] << ", "   
		 	<< this_fe->m_location[1] << ", "   
		 	<< this_fe->m_location[2] << ") " 

			<< ",  t=( " 
			<< this_fe->m_tangent[0] << ", "   
			<< this_fe->m_tangent[1] << ", "   
			<< this_fe->m_tangent[2]
			<< " )" << std::endl;
	}

	Vector3f smoothed{ 0.0f, 0.0f, 0.0f};

	for( auto edge_iter = gn->m_edges.begin(); edge_iter != gn->m_edges.end(); ++edge_iter ) {

		const GraphNode * neighbouring_node = std::get<2>(*edge_iter);
		FieldElement * neighbour_fe = (FieldElement *) neighbouring_node->m_data;


		if( m_tracing_enabled ) {
			std::cout << "    consider neighbour : l=( " 
				<< neighbour_fe->m_location[0] << ", "   
			 	<< neighbour_fe->m_location[1] << ", "   
			 	<< neighbour_fe->m_location[2] << ") " 

				<< ",  t=( " 
				<< neighbour_fe->m_tangent[0] << ", "   
				<< neighbour_fe->m_tangent[1] << ", "   
				<< neighbour_fe->m_tangent[2]
				<< " )" << std::endl;
		}

		Vector3f best = best_rosy_vector_for( 
			this_fe->m_tangent,
			this_fe->m_normal,
			0, 
			neighbour_fe->m_tangent, 
			neighbour_fe->m_normal);

		smoothed = smoothed + best;

		if( m_tracing_enabled ) {
			std::cout << "    best fit was ( " 
				<< best[0] << ", "   
				<< best[1] << ", "   
				<< best[2] << ") " << std::endl;

			std::cout << "    wip tangent is now ( " 
				<< smoothed[0] << ", "   
				<< smoothed[1] << ", "   
				<< smoothed[2] << ") " << std::endl;
		}
	}
	smoothed = reproject_to_tangent_space( smoothed, this_fe->m_normal );
	if( m_tracing_enabled ) {
		std::cout << "    new tangent is ( " 
			<< smoothed[0] << ", "   
			<< smoothed[1] << ", "   
			<< smoothed[2] << ") " << std::endl;
	}
	return smoothed;
}



/**
 * Given an arbitrary vector v, project it into the plane whose normal is given as n
 * also unitize it.
 * @param v The vector
 * @param n The normal
 * @return a unit vector in the tangent plane
 */
Eigen::Vector3f reproject_to_tangent_space( const Eigen::Vector3f& v, const Eigen::Vector3f& n) {
	using namespace Eigen;

	Vector3f error = v.dot( n ) * n;
	Vector3f projection = (v - error).normalized();
	return projection;
}


/**
 * Return vector of elements */
const std::vector<const FieldElement *> Field::elements( ) const {
	std::vector<const FieldElement *> elements;
	for( auto node_iter = m_graph->m_data_to_node_map.begin(); node_iter != m_graph->m_data_to_node_map.end(); ++node_iter ) {
		const FieldElement * fe = (FieldElement *) (*node_iter).first;

		elements.push_back( fe );
	}
	return elements;
}

/**
 * @param targetVector The vector we're trying to match
 * @param normal The normal about which to rotate the sourceVector
 * @param sourceVector the vector to be matched
 * @return the best fitting vector (i.e. best multiple of PI/2 + angle)
 * 
 */
Eigen::Vector3f best_rosy_vector_by_dot_product( const Eigen::Vector3f& targetVector, 
									  const Eigen::Vector3f& targetNormal, 
									  int targetK, 
									  const Eigen::Vector3f& sourceVector, 
									  const Eigen::Vector3f& sourceNormal ) {
	using namespace Eigen;

	Vector3f effectiveTarget = vectorByRotatingOAroundN( targetVector, targetNormal, targetK );
	Vector3f best{ sourceVector };
	float bestDotProduct = effectiveTarget.dot( sourceVector );

	for( int k=1; k<4; ++k ) {
		Vector3f testVector = vectorByRotatingOAroundN( sourceVector, sourceNormal, k );

		float dp = effectiveTarget.dot( testVector );
		if( dp > bestDotProduct) {
			bestDotProduct = dp;
			best = testVector;
		}
	}
	return best;
}

/**
 * @param targetVector The vector we're trying to match
 * @param normal The normal about which to rotate the sourceVector
 * @param sourceVector the vector to be matched
 * @return the best fitting vector (i.e. best multiple of PI/2 + angle)
 * 
 */
Eigen::Vector3f best_rosy_vector_for( const Eigen::Vector3f& targetVector, 
									  const Eigen::Vector3f& targetNormal, 
									  int targetK, 
									  const Eigen::Vector3f& sourceVector, 
									  const Eigen::Vector3f& sourceNormal ) {
	using namespace Eigen;

	Vector3f effectiveTarget = vectorByRotatingOAroundN( targetVector, targetNormal, targetK );
	Vector3f best{ 0.0f, 0.0f, 0.0f };
	float bestAngle = 3 * M_PI;

	for( int k=0; k<4; ++k ) {
		Vector3f testVector = vectorByRotatingOAroundN( sourceVector, sourceNormal, k );

		float theta = angleBetweenVectors( effectiveTarget, testVector );
		if( fabsf(theta) < fabsf(bestAngle) ) {
			bestAngle = theta;
			best = testVector;
		}
	}
	return best;
}

/**
 * @param targetVector The vector we're trying to match
 * @param normal The normal about which to rotate the sourceVector
 * @param sourceVector the vector to be matched
 * @return the best fitting vector (i.e. best multiple of PI/2 + angle)
 * 
 */
Eigen::Vector3f best_rosy_vector_and_kl( const Eigen::Vector3f& target_vector, const Eigen::Vector3f& target_normal, int& k_ij, 
									  	 const Eigen::Vector3f& source_vector, const Eigen::Vector3f& source_normal, int& k_ji ) {
	using namespace Eigen;

	float best_dot_product	= -2.0f;
	Vector3f best_vector{ 0.0f, 0.0f, 0.0f };

	int k =0, l=0;
	for( k =0; k < 4; ++k ) {

		Vector3f test_target = vectorByRotatingOAroundN( target_vector, target_normal, k );

		for( l =0; l < 4; ++l ) {
			Vector3f test_source = vectorByRotatingOAroundN( source_vector, source_normal, l );

			float dp = test_target.dot( test_source );
			// Force a new solution to be better than an existing one
			if( dp > (best_dot_product + EPSILON) ) {
				best_dot_product = dp;
				best_vector = test_source;
			}
		}
	}

	k_ij = k;
	k_ji = l;
	return best_vector;
}

/**
 * Dump the field to stdout. For each point in the field dump the neighbour locations
 * and the field tangent
 */
void Field::dump(  ) const {
	for( auto pair_iter = m_graph->m_data_to_node_map.begin();
		      pair_iter != m_graph->m_data_to_node_map.end();
		      ++pair_iter ) {

		FieldElement *fe = (FieldElement *)((*pair_iter).first);
		GraphNode *gn    = (GraphNode*)    ((*pair_iter).second);

		std::cout << "locn    (" << fe->m_location[0] << "," << fe->m_location[1] << "," << fe->m_location[2] << ")" << std::endl;
		std::cout << "tangent (" << fe->m_tangent[0] << "," << fe->m_tangent[1] << "," << fe->m_tangent[2] << std::endl;
		std::cout << "neighbours " << std::endl;
		for( auto edge_iter  = gn->m_edges.begin();
			      edge_iter != gn->m_edges.end();
			      ++edge_iter ) {
			GraphNode * gn = std::get<2>(*edge_iter);
			FieldElement * fe_n = (FieldElement *) (gn->m_data);

			std::cout << "      " << fe_n->m_location[0] << "," << fe_n->m_location[1] << "," << fe_n->m_location[2] << std::endl;
		}
	}
}