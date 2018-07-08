#include <Field/Field.h>
#include <Element/Element.h>
#include <RoSy/RoSy.h>
#include <Eigen/Core>
#include <Eigen/Geometry>
#include <vector>
#include <pcl/kdtree/kdtree_flann.h>
#include <pcl/io/obj_io.h>
#include <FileUtils/FileUtils.h>


//#define TRACE  1

#include <iostream>
#include <queue>
#include <set>

const int MAX_ITERS_PER_TIER = 20;
const float CONVERGENCE_THRESHOLD = 0.5f;

using Graph = animesh::Graph<FieldElement *, void *>;
using GraphNode = typename animesh::Graph<FieldElement *, void *>::GraphNode;

Field::~Field( ) {
	delete m_graph;
}

FieldElement * field_element_from_point( const pcl::PointNormal& point ) {
	FieldElement * fe = new FieldElement( 
		Eigen::Vector3f{ point.x, point.y, point.z},
		Eigen::Vector3f{ point.normal_x, point.normal_y, point.normal_z},
		Eigen::Vector3f::Zero());
	return fe;
}

bool operator == (const pcl::PointNormal& point1, const pcl::PointNormal& point2 ) {
	return (point1.x == point2.x) &&
		(point1.y == point2.y) &&
		(point1.z == point2.z) &&
		(point1.normal_x == point2.normal_x) &&
		(point1.normal_y == point2.normal_y) &&
		(point1.normal_z == point2.normal_z);
}

/*
 * Comparator for pcl::PointNormal instances
 */
struct cmpPointNormal {
	bool operator()(const pcl::PointNormal& a, const pcl::PointNormal& b) const {
		if( a.x < b.x ) return true;
		if( a.x > b.x ) return false;
		if( a.y < b.y ) return true;
		if( a.y > b.y ) return false;
		if( a.z < b.z ) return true;
		if( a.z > b.z ) return false;

		if( a.normal_x < b.normal_x ) return true;
		if( a.normal_x > b.normal_x ) return false;
		if( a.normal_y < b.normal_y ) return true;
		if( a.normal_y > b.normal_y ) return false;
		if( a.normal_z < b.normal_z ) return true;

		return false;
    }
};

/**
 * Find the GN corresponding to a point 
 * or else throw an exception
 */
GraphNode * find_node_or_throw( const std::map<pcl::PointNormal, GraphNode *, cmpPointNormal>& map, 
	const pcl::PointNormal& point ) {

		auto find_it = map.find(point);
		if( find_it == map.end() )
			throw std::runtime_error( "No FE found for point" );
	return find_it->second;
}


/**
 * Load an obj file into a point cloud
 */
pcl::PointCloud<pcl::PointNormal>::Ptr load_pointcloud_from_obj( const std::string& file_name ) {
    if( file_name.size() == 0 ) 
        throw std::invalid_argument( "Missing file name" );

    bool is_directory;
    if (!file_exists(file_name, is_directory ) )
        throw std::runtime_error( "File not found: " + file_name );

    pcl::PointCloud<pcl::PointNormal>::Ptr cloud (new pcl::PointCloud<pcl::PointNormal>);
    if( pcl::io::loadOBJFile(file_name, *cloud) == -1) {
        PCL_ERROR ("Couldn't read file\n");
        return nullptr;
    }
    return cloud;
}


/**
 * Construct a field from an OBJ file
 */
Field * load_field_from_obj_file( const std::string& file_name, int k, float with_scaling, bool trace ) {
	std::cout << "Loading from file " << file_name << std::endl;

	// Load the point cloud from file
	pcl::PointCloud<pcl::PointNormal>::Ptr cloud = load_pointcloud_from_obj(file_name);
	if( !cloud ) 
		return nullptr;

	// Scale points
	std::cout << "scaling points by " << with_scaling << std::endl;
	for( auto iter = cloud->points.begin(); iter != cloud->points.end(); ++iter ) {
		(*iter).x *= with_scaling;
		(*iter).y *= with_scaling;
		(*iter).z *= with_scaling;
	}

	std::cout << "building graph with " << k<< " nearest neighbours." << std::endl;
	return new Field( cloud, k, trace );
}

/**
 * Construct a field given a point cloud
 */
Field::Field( const pcl::PointCloud<pcl::PointNormal>::Ptr cloud, int k, bool tracing_enabled ) {
	m_tracing_enabled = tracing_enabled;
	
	// First make an empty Graph
    m_graph = new Graph( FieldElement::mergeFieldElements, FieldElement::propagateFieldElements );

	// Next add all the points to it
	std::map<pcl::PointNormal, GraphNode*, cmpPointNormal> point_to_gn_map;
	size_t points_added = 0;
    for( auto point : *cloud ) {

		FieldElement * fe = field_element_from_point( point );
		GraphNode * gn = m_graph->add_node( fe );

		// Keep a mapping from point to gn
		point_to_gn_map[ point ] = gn;

		++points_added;
	}
	if( m_tracing_enabled )
		std::cout<< "Added " << points_added << " points" << std::endl;



    // Set up for kNN
    pcl::KdTreeFLANN<pcl::PointNormal> kdtree;
    kdtree.setInputCloud (cloud);
    std::vector<int> pointIdxNKNSearch(k);
    std::vector<float> pointNKNSquaredDistance(k);

	// Keep track of points visited
	std::set<pcl::PointNormal, cmpPointNormal> visited;

	// For each point, construct edges
    for( auto this_point : *cloud ) {
		if( m_tracing_enabled )
			std::cout<< "Processing point " << this_point << std::endl;

		// Look up GN
		GraphNode * this_gn = find_node_or_throw( point_to_gn_map, this_point);

    	// Now find neighbours
    	pointIdxNKNSearch.clear();
    	pointNKNSquaredDistance.clear();
        if ( kdtree.nearestKSearch (this_point, k, pointIdxNKNSearch, pointNKNSquaredDistance) > 0 ) {

			if( m_tracing_enabled ) {
				std::cout<< "  Found " << pointIdxNKNSearch.size() << " neighbours :" << std::endl;
	            for ( size_t i : pointIdxNKNSearch ) {
	            	pcl::PointNormal neighbour_point = cloud->points[ i ];
					std::cout<< "    Point: " << neighbour_point << ";  GN " << find_node_or_throw( point_to_gn_map, neighbour_point) << std::endl;
	            }
			}


        	// ... and add edges to them
			if( m_tracing_enabled )
				std::cout<< "  Looking at neighbours" << std::endl;

            for ( size_t i : pointIdxNKNSearch ) {
            	pcl::PointNormal neighbour_point = cloud->points[ i ];
				if( m_tracing_enabled )
					std::cout<< "    next point is " << neighbour_point << std::endl;

            	// Provided the eighbour isn't this point
            	if( ! (neighbour_point == this_point ) ) {
            		// If the neighbour GN is not found... throw
					GraphNode * neighbour_gn = find_node_or_throw( point_to_gn_map, neighbour_point);

					if( m_tracing_enabled )
						std::cout<< "    Adding edge to " << neighbour_point << std::endl;
            	    m_graph->add_edge( this_gn, neighbour_gn, 1.0f, nullptr );
            	}
            	else {
					if( m_tracing_enabled )
						std::cout<< "    Ignoring me as my own neighbour" << std::endl;
            	}
        	}
        }
	}

    // Graph is built
    std::cout << "Done" << std::endl;

    randomise_tangents( );
    m_top_graph = m_graph;
    generate_hierarchy( 100 );}


/**
 * Generate the hierarchical grah by repeatedly simplifying until there are e.g. less than 20 nodes
 */
void Field::generate_hierarchy( size_t max_nodes ) {
	if( m_graph != m_top_graph)
		throw std::runtime_error( "Hierarchy already generated" );

	if( m_tracing_enabled ) {
		std::cout<< "Generating graph hierarchy, max "  << max_nodes << " nodes" << std::endl;
		std::cout<< "  Start :"  << m_graph->num_nodes() << " nodes, " << m_graph->num_edges() << " edges" << std::endl;
	}

	bool done = false;
	m_num_tiers = 1;
	while ( !done && (m_top_graph->num_nodes( ) > max_nodes ) ) {
		Graph * next_graph = m_top_graph->simplify( );
		if( next_graph == nullptr) {
			done = true;
		}
		else {
			m_top_graph = next_graph;
			m_num_tiers ++;
		}

		if( m_tracing_enabled )
			std::cout<< "  Nodes :"  << next_graph->num_nodes() << ", Edges :" << next_graph->num_edges() << std::endl;
	}
}

void Field::randomise_tangents( ) {
	// Initialise field tangents to random values
	for( auto node_iter = m_graph->nodes().begin(); node_iter != m_graph->nodes().end(); ++node_iter ) {
		GraphNode * gn = *node_iter;

		Eigen::Vector3f random = Eigen::VectorXf::Random(3);
		random = random.cross( gn->data()->m_normal ).normalized( );
		gn->data()->m_tangent = random;
	}
}


/**
 * @return the size of the ifled
 */
std::size_t Field::size() const {
	return m_graph->num_nodes();
}

/**
 * @return the smoothness of the entire Field
 */
float Field::calculate_error( Graph * tier ) const {
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
float Field::calculate_error_for_node( Graph * tier, GraphNode * gn ) const {
	float error = 0.0f;

	FieldElement * this_fe = (FieldElement *) gn->data();

	std::vector<GraphNode *> neighbours = tier->neighbours( gn );

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

/**
 * Smooth the field
 */
void Field::smooth_completely() {
	m_smoothing_current_tier = m_top_graph;
	m_smoothing_tier_index = 1;
	m_smoothing_started_new_tier = true;
	m_smoothing_last_error = calculate_error( m_smoothing_current_tier );
	m_is_smoothing = true;
	if( m_tracing_enabled ) 
		std::cout << "Starting smooth. Error : " << m_smoothing_last_error << std::endl;

	do {

		// If we're starting a new tier...
		if( m_smoothing_started_new_tier ) {
			if( m_tracing_enabled ) 
				std::cout << "  Level " << m_smoothing_tier_index << std::endl;

			m_smoothing_iterations_this_tier = 0;
			m_smoothing_started_new_tier = false;
		}

		if( smooth_tier( m_smoothing_current_tier ) ) {
			// Converged. If there's another tier, do it
			if( m_smoothing_current_tier != m_graph ) {
				m_smoothing_current_tier = m_smoothing_current_tier -> propagate_down( );
				m_smoothing_tier_index ++;
				m_smoothing_started_new_tier = true;
			}

			// Otherwise, done
			else {
				m_is_smoothing = false;
			}
		}
	} while( m_is_smoothing );
}

/**
 * Smooth the field
 */
void Field::smooth() {
	// If not smoothing, start
	if( ! m_is_smoothing ) {
		m_is_smoothing = true;
		m_smoothing_current_tier = m_top_graph;
		m_smoothing_tier_index = 1;
		m_smoothing_started_new_tier = true;
		m_smoothing_last_error = calculate_error( m_smoothing_current_tier );
		if( m_tracing_enabled ) 
			std::cout << "Starting smooth. Error : " << m_smoothing_last_error << std::endl;
	}

	// If we're starting a new tier...
	if( m_smoothing_started_new_tier ) {
		if( m_tracing_enabled ) 
			std::cout << "  Level " << m_smoothing_tier_index << std::endl;

		m_smoothing_iterations_this_tier = 0;
		m_smoothing_started_new_tier = false;
	}


	if( smooth_tier( m_smoothing_current_tier ) ) {
		// Converged. If there's another tier, do it
		if( m_smoothing_current_tier != m_graph ) {
			m_smoothing_current_tier = m_smoothing_current_tier -> propagate_down( );
			m_smoothing_tier_index ++;
			m_smoothing_started_new_tier = true;
		}

		// Otherwise, done
		else {
			m_is_smoothing = false;
		}
	}
}

/**
 * Smooth the current tier of the hierarchy by repeatedly smoothing until the error doesn't change
 * significantly.
 */
bool Field::smooth_tier( Graph * tier ) {

	float new_error = smooth_once( tier );
	m_smoothing_iterations_this_tier ++;

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
 * Smooth the field once, applying smoothing to each node
 * @return the largest error in tangent
 */
float Field::smooth_once( Graph * tier ) {
	using namespace Eigen;
	using namespace std;

	if( m_tracing_enabled )
		cout << "    smooth_once" << endl;

	// Extract map keys into vector and shuffle
	std::vector<GraphNode *> nodes = tier->nodes();
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
	return calculate_error( tier );
}


/**
 * Smooth the specified node
 * @return The new vector.
 */
Eigen::Vector3f Field::calculate_smoothed_node( Graph * tier, GraphNode * gn ) const {
	using namespace Eigen;

	FieldElement * this_fe = (FieldElement *) gn->data();
	// if( m_tracing_enabled ) 
	// 	trace_node( "smooth_node", this_fe);

	Vector3f sum = this_fe->m_tangent;
	float weight = 0;

	// For each edge from this node
	std::vector<GraphNode *> neighbours = tier->neighbours( gn );
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

/**
 * Set all the tangents in the field to specific values.
 * @param new_tangents The new tangents
 * @return The difference between the old and new values
 */
void Field::set_tangents( const std::vector<const Eigen::Vector3f>& new_tangents ) {
	using namespace Eigen;

	if( new_tangents.size() != m_graph->num_nodes() ) 
		throw std::invalid_argument( "Must be one tangent for each node");

	auto tan_iter = new_tangents.begin();
	for( auto	node_iter = m_graph->nodes().begin(); 
				node_iter != m_graph->nodes().end(); ++
				node_iter, ++tan_iter ) {
		FieldElement * fe = (*node_iter)->data();

		const Vector3f current_tangent = fe->m_tangent;
		const Vector3f new_tangent = (*tan_iter);
		fe->m_tangent = new_tangent; //(0.5 * fe->m_tangent + 0.5 * new_tangent).normalized();
	}
}

/**
 * Return vector of elements
 */
const std::vector<const FieldElement *> Field::elements( int tier ) const {
	std::vector<const FieldElement *> elements;

	Graph * g = graph_at_tier(tier);
	for( auto node : g->nodes() ) {
		elements.push_back( node->data() );
	}
	return elements;
}

/**
 * Dump the field to stdout. For each point in the field dump the neighbour locations
 * and the field tangent
 */
void Field::dump(  ) const {
	for( auto node_iter = m_graph->nodes().begin();
		      node_iter != m_graph->nodes().end();
		      ++node_iter ) {

		using GraphNode = animesh::Graph<FieldElement *, void*>::GraphNode;

		GraphNode    *gn = (*node_iter);
		FieldElement *fe = gn->data();

		std::cout << "locn    (" << fe->m_location[0] << "," << fe->m_location[1] << "," << fe->m_location[2] << ")" << std::endl;
		std::cout << "tangent (" << fe->m_tangent[0] << "," << fe->m_tangent[1] << "," << fe->m_tangent[2] << std::endl;
		std::cout << "neighbours " << std::endl;

		std::vector<GraphNode *> neighbours = m_graph->neighbours( gn );
		for( auto neighbour_iter  = neighbours.begin();
			      neighbour_iter != neighbours.end();
			      ++neighbour_iter ) {
			FieldElement * fe_n = (*neighbour_iter)->data();
			std::cout << "      " << fe_n->m_location[0] << "," << fe_n->m_location[1] << "," << fe_n->m_location[2] << std::endl;
		}
	}
}

void Field::trace_node( const std::string& prefix, const FieldElement * this_fe ) const {
	std::cout << prefix << this_fe << std::endl;
}

void Field::trace_vector( const std::string& prefix, const Eigen::Vector3f& vector ) const {
	std::cout << prefix << vector << std::endl;
}

std::ostream& operator<<( std::ostream& os, const FieldElement& fe) {
	os	<< "( l=( " 
		<< fe.m_location[0] << ", "   
 		<< fe.m_location[1] << ", "   
 		<< fe.m_location[2] << "), t=(" 
		<< fe.m_tangent[0] << ", "   
 		<< fe.m_tangent[1] << ", "   
 		<< fe.m_tangent[2] << ")";
 	return os;
}

std::ostream& operator<<( std::ostream& os, const Eigen::Vector3f& vector) {
	os  << vector[0] << ", "   
	 	<< vector[1] << ", "   
	 	<< vector[2] << ")";
 	return os;
}
