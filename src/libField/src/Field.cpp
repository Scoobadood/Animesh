#include <Field/Field.h>
#include <Eigen/Core>
#include <Eigen/Geometry>
#include <pcl/kdtree/kdtree_flann.h>
#include <pcl/io/obj_io.h>
#include <FileUtils/FileUtils.h>

#include <iostream>
#include <queue>
#include <set>
#include <vector>

//#define TRACE  1

using FieldGraph = animesh::Graph<FieldElement *, void *>;
using FieldGraphNode = typename animesh::Graph<FieldElement *, void *>::GraphNode;

/* ******************************************************************************************
 * **
 * **  Utility functions
 * **
 * ******************************************************************************************/
/**
 * Construct a FieldElement given a point
 */
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
FieldGraphNode * find_node_or_throw( const std::map<pcl::PointNormal, FieldGraphNode *, cmpPointNormal>& map, 
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

/* ******************************************************************************************
 * **
 * **  Field Element Methods
 * **
 * ******************************************************************************************/


/**
 * Merge FieldElements when simplifying a graph. Each FieldElement has a normal, tangent and location.
 * When merging we do the following:
 * Locations are averaged
 * Normals are averaged
 * Tangents are randomised, made perpendicular to the normal and unitised
 */
FieldElement * FieldElement::mergeFieldElements ( const FieldElement * const fe1, const FieldElement * const fe2 ) {
	using namespace Eigen;

	Vector3f new_location = (fe1->m_location + fe2->m_location) / 2.0;
	Vector3f new_normal   = (fe1->m_normal + fe2->m_normal).normalized();
	Vector3f new_tangent  = Eigen::Vector3f::Random().cross( new_normal ).normalized();

	FieldElement *fe = new FieldElement( new_location, new_normal, new_tangent );
	return fe;
}

/**
 * When simplifying the graph, propagate the changes from parent to child.
 * by just copying it.
 */
FieldElement * FieldElement::propagateFieldElements ( const FieldElement * const parent, const FieldElement * const child ) {
	using namespace Eigen;

	// Take the tangent from the parent and reproject into the child's tangent space and normalise
	Vector3f error = parent->m_tangent.dot( child->m_normal ) * child->m_normal;
	Vector3f new_tangent = (parent->m_tangent - error).normalized( );
	return new FieldElement( child->m_location, child->m_normal, new_tangent );
}


/* ******************************************************************************************
 * **
 * **  
 * **
 * ******************************************************************************************/

/**
 * Construct a field given a point cloud
 */
Field::Field( const pcl::PointCloud<pcl::PointNormal>::Ptr cloud, int k, bool tracing_enabled ) {
	m_tracing_enabled = tracing_enabled;

	// Clean up after last object if appropriate
	clear_up( );
	
	// First make an empty FieldGraph
	m_graph_hierarchy.push_back( new FieldGraph( ) );

	// Next add all the points to it
	std::map<pcl::PointNormal, FieldGraphNode*, cmpPointNormal> point_to_gn_map;
	size_t points_added = 0;
    for( auto point : *cloud ) {

		FieldElement * fe = field_element_from_point( point );
		FieldGraphNode * gn = m_graph_hierarchy[0] ->add_node( fe );

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
		FieldGraphNode * this_gn = find_node_or_throw( point_to_gn_map, this_point);

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
					FieldGraphNode * neighbour_gn = find_node_or_throw( point_to_gn_map, neighbour_point);

					if( m_tracing_enabled )
						std::cout<< "    Adding edge to " << neighbour_point << std::endl;
            	    m_graph_hierarchy[0]->add_edge( this_gn, neighbour_gn, 1.0f, nullptr );
            	}
            	else {
					if( m_tracing_enabled )
						std::cout<< "    Ignoring me as my own neighbour" << std::endl;
            	}
        	}
        }
	}

    // FieldGraph is built
    std::cout << "Done" << std::endl;

    randomise_tangents( );
    generate_hierarchy( 100 );
}

/**
 * Destructor for fields
 */
Field::~Field( ) {
	clear_up( );
}



void Field::clear_up( ) {
	for( auto g : m_graph_hierarchy ) {
		delete g;
	}
	m_graph_hierarchy.clear();
	m_mapping_hierarchy.clear();
	m_is_smoothing = false;
	m_smoothing_started_new_tier = false;
	m_smoothing_last_error = 0.0f;
	m_smoothing_iterations_this_tier = 0;
	m_smoothing_tier_index = 0;
	m_smoothing_current_tier = nullptr;
}


/**
 * Generate a hierarchical graph by repeatedly simplifying until there are e.g. less than 20 nodes
 * Stash the graphs and mappings into vectors.
 */
void Field::generate_hierarchy( size_t max_nodes ) {
	if( m_graph_hierarchy.size() == 0 )
		throw std::runtime_error( "No base graph to generate hierarchy" );
	
	if( m_graph_hierarchy.size() > 1 )
		throw std::runtime_error( "Hierarchy already generated" );

	if( m_tracing_enabled ) {
		std::cout<< "Generating graph hierarchy, max "  << max_nodes << " nodes" << std::endl;
		std::cout<< "  Start :"  << m_graph_hierarchy[0]->num_nodes() << " nodes, " << m_graph_hierarchy[0]->num_edges() << " edges" << std::endl;
	}

	bool done = false;

	FieldGraphSimplifier * s = new FieldGraphSimplifier(FieldElement::mergeFieldElements, FieldElement::propagateFieldElements);
	FieldGraph * g = m_graph_hierarchy[0];
	while ( !done && (g->num_nodes( ) > max_nodes ) ) {
		if (g->num_edges() == 0 ) {
			done = true;
		}
		else {
			std::pair<FieldGraph *,FieldGraphMapping> p = s->simplify( g );
			m_graph_hierarchy.push_back( p.first );
			m_mapping_hierarchy.push_back( p.second );
			g = p.first;
		}

		if( m_tracing_enabled )
			std::cout<< "  Nodes :"  << g->num_nodes() << ", Edges :" << g->num_edges() << std::endl;
	}
}

void Field::randomise_tangents( ) {
	// Initialise field tangents to random values
	for( auto gn : m_graph_hierarchy[0]->nodes() ) {

		Eigen::Vector3f random = Eigen::VectorXf::Random(3);
		random = random.cross( gn->data()->m_normal ).normalized( );
		gn->data()->m_tangent = random;
	}
}


/**
 * @return the size of the ifled
 */
std::size_t Field::size() const {
	return m_graph_hierarchy[0]->num_nodes();
}


/**
 * Return vector of elements
 */
const std::vector<const FieldElement *> Field::elements( int tier ) const {
	std::vector<const FieldElement *> elements;

	FieldGraph * g = graph_at_tier(tier);
	for( auto node : g->nodes() ) {
		elements.push_back( node->data() );
	}
	return elements;
}

/**
 * @Return the nth graph in the hierarchy where 0 is base.
 */
FieldGraph * Field::graph_at_tier( size_t tier ) const {
	return m_graph_hierarchy[tier];
}


