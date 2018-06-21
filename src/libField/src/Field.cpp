#include <Field/Field.h>
#include <Element/Element.h>
#include <RoSy/RoSy.h>
#include <Eigen/Core>
#include <Eigen/Geometry>
#include <vector>
#include <pcl/kdtree/kdtree_flann.h>

//#define TRACE  1

#include <iostream>
#include <queue>
#include <set>

using Graph = animesh::Graph<FieldElement *, void *>;
using GraphNode = typename animesh::Graph<FieldElement *, void *>::GraphNode;

Field::~Field( ) {
	delete m_graph;
}


void Field::init( const GraphBuilder<void*> * const graph_builder, const std::vector<Element>& elements ) {
	m_graph = graph_builder->build_graph_for_elements( elements );

	// Initialise field tangents to random values
	for( auto node_iter = m_graph->nodes().begin(); node_iter != m_graph->nodes().end(); ++node_iter ) {
		Eigen::Vector3f random = Eigen::VectorXf::Random(3);
		random = random.cross( (*node_iter)->data()->m_normal ).normalized( );
		(*node_iter)->data()->m_tangent = random;
	}
}


/**
 * Construct the field using a graph builder and some elements
 */
Field::Field( const GraphBuilder<void*> * const graph_builder, const std::vector<Element>& elements ) {
	init( graph_builder, elements);
}

FieldElement * field_element_from_point( const pcl::PointNormal& point ) {
	FieldElement * fe = new FieldElement( 
		Eigen::Vector3f{ point.x * 100, point.y * 100, point.z * 100},
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

		float v1x = (a.x + a.normal_x);
		float v1y = (a.y + a.normal_y);
		float v1z = (a.z + a.normal_z);

		float v2x = (b.x + b.normal_x);
		float v2y = (b.y + b.normal_y);
		float v2z = (b.z + b.normal_z);

		return (v1x*v1x+v1y*v1y+v1z*v1z) < (v2x*v2x+v2y*v2y+v2z*v2z);
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
 * Construct a field given a point cloud
 */
Field::Field( const pcl::PointCloud<pcl::PointNormal>::Ptr cloud, int k, bool tracing_enabled ) {

	m_tracing_enabled = tracing_enabled;
	
	// First make an empty Graph
    m_graph = new Graph( FieldElement::mergeFieldElements );

	// Next add all the points to it
	std::map<pcl::PointNormal, GraphNode*, cmpPointNormal> point_to_gn_map;
	size_t points_added = 0;
    for( auto it = cloud->begin(); it != cloud->end(); ++it ) {
    	pcl::PointNormal point = *it;
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
    for( auto it = cloud->begin(); it != cloud->end(); ++it ) {
    	pcl::PointNormal this_point = *it;
		if( m_tracing_enabled )
			std::cout<< "Processing point " << this_point << std::endl;

		// Look up GN
		GraphNode * this_gn = find_node_or_throw( point_to_gn_map, this_point);

    	// Now find neighbours
    	pointIdxNKNSearch.clear();
    	pointNKNSquaredDistance.clear();
        if ( kdtree.nearestKSearch (this_point, k, pointIdxNKNSearch, pointNKNSquaredDistance) > 0 ) {

        	// ... and add edges to them
			if( m_tracing_enabled )
				std::cout<< "  Looking at neighbours" << std::endl;

            for (size_t i = 0; i < pointIdxNKNSearch.size (); ++i) {
            	pcl::PointNormal neighbour_point = cloud->points[ pointIdxNKNSearch[i] ];

            	// Provided the eighbour isn't this point
            	if( ! (neighbour_point == this_point ) ){
            		// If the neighbour GN is not found... throw
					GraphNode * neighbour_gn = find_node_or_throw( point_to_gn_map, neighbour_point);

            	    m_graph->add_edge( this_gn, neighbour_gn, 1.0f, nullptr );
					if( m_tracing_enabled )
						std::cout<< "    Add neighbour " << neighbour_point << std::endl;
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

    generate_hierarchy( 20 );
}

/**
 * Generate the hierarchical grah by repeatedly simplifying until there are e.g. less than 20 nodes
 */
void Field::generate_hierarchy( size_t max_nodes ) {
	if( m_tracing_enabled )
		std::cout<< "Generating graph hierarchy. Starting at " << m_graph->num_nodes() << " and going to " << max_nodes << " nodes maximum" << std::endl;

	bool done = false;
	while ( !done && (m_graph->num_nodes( ) > max_nodes ) ) {
		if( m_tracing_enabled )
			std::cout<< "    Now " << m_graph->num_nodes() << " nodes" << std::endl;
		Graph * next_graph = m_graph->simplify( );
		if( next_graph == nullptr) {
			done = true;
		}
		else {
			m_graph = next_graph;
		}
	}
	if( m_tracing_enabled )
		std::cout<< "    Final nodes :  " << m_graph->num_nodes() << std::endl;
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
float Field::error( ) const {
	// E(O, k) :=      (oi, Rso (oji, ni, kij ))
	// For each node
	float error = 0.0f;
	for( auto node_iter = m_graph->nodes().begin(); 
		node_iter != m_graph->nodes().end(); 
		++node_iter ) {

		error += get_error_for_node( *node_iter );
	}
	return error;
}

/**
 * @return the smoothness of one node
 */
float Field::get_error_for_node( 
	animesh::Graph<FieldElement *, void*>::GraphNode* gn ) const {
	using GraphNode = typename animesh::Graph<FieldElement *, void *>::GraphNode;
	float error = 0.0f;

	FieldElement * this_fe = (FieldElement *) gn->data();

	std::vector<GraphNode *> neighbours = m_graph->neighbours( gn );

	for( auto n = neighbours.begin(); n != neighbours.end(); ++n ) {

		FieldElement * neighbour_fe = (*n)->data();

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
 * Smooth the field once, applying smoothing to each node
 * @return the largest error in tangent
 */
void Field::smooth_once( ) {
	using namespace Eigen;
	using namespace std;
	using GraphNode = animesh::Graph<FieldElement *, void *>::GraphNode;

	if( m_tracing_enabled )
		cout << "smooth_once" << endl;

	// Extract map keys into vector and shuffle
	std::vector<GraphNode *> nodes = m_graph->nodes();
	random_shuffle ( nodes.begin(), nodes.end() ); 

	// Iterate over permute, look up key, lookup fe and smooth
	vector<const Vector3f> new_tangents;
	for( auto node_iter = nodes.begin(); node_iter != nodes.end(); ++node_iter ) {
		Vector3f new_tangent = smooth_node( *node_iter );
		new_tangents.push_back( new_tangent );
	}

	// Now update all of the nodes
	auto tan_iter = new_tangents.begin();
	for( auto node_iter = nodes.begin(); node_iter != nodes.end(); ++node_iter ) {
		(*node_iter)->data()->m_tangent = (*tan_iter);
	}
}


/**
 * Smooth the specified node (and neighbours)
 * @return The new vector.
 */
Eigen::Vector3f Field::smooth_node( animesh::Graph<FieldElement *, void*>::GraphNode * gn ) const {
	using namespace Eigen;
	using GraphNode = animesh::Graph<FieldElement *, void *>::GraphNode;

	FieldElement * this_fe = (FieldElement *) gn->data();
	if( m_tracing_enabled ) 
		trace_node( "smooth_node", this_fe);

	Vector3f sum = this_fe->m_tangent;
	float weight = 0;

	// For each edge from this node
	std::vector<GraphNode *> neighbours = m_graph->neighbours( gn );
	for( auto neighbour_iter = neighbours.begin(); neighbour_iter != neighbours.end(); ++neighbour_iter ) {

		// Get the adjacent FieldElement
		FieldElement * neighbour_fe = (*neighbour_iter)->data();
		if( m_tracing_enabled ) trace_node( "    consider neighbour", neighbour_fe );

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
const std::vector<const FieldElement *> Field::elements( ) const {
	std::vector<const FieldElement *> elements;
	for( auto node_iter = m_graph->nodes().begin(); node_iter != m_graph->nodes().end(); ++node_iter ) {
		const FieldElement * fe = (*node_iter)->data();

		elements.push_back( fe );
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
	std::cout << prefix 
		<< "( l=( " 
		<< this_fe->m_location[0] << ", "   
	 	<< this_fe->m_location[1] << ", "   
	 	<< this_fe->m_location[2] << "), t=(" 
		<< this_fe->m_tangent[0] << ", "   
	 	<< this_fe->m_tangent[1] << ", "   
	 	<< this_fe->m_tangent[2] << ")" 
	 	<< std::endl;
}

void Field::trace_vector( const std::string& prefix, const Eigen::Vector3f& vector ) const {
	std::cout << prefix  
		<< vector[0] << ", "   
	 	<< vector[1] << ", "   
	 	<< vector[2] << ") " 
	 	<< std::endl;
}
