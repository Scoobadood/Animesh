#include <Field/Field.h>
#include <Field/FieldElement.h>

#include <Eigen/Core>
#include <Eigen/Geometry>
#include <pcl/kdtree/kdtree_flann.h>

#include <iostream>
#include <queue>
#include <set>
#include <vector>

namespace animesh {

using FieldGraph = animesh::Graph<FieldElement *, void *>;
using FieldGraphNode = typename animesh::Graph<FieldElement *, void *>::GraphNode;

/* ******************************************************************************************
 * **
 * **  Utility functions
 * **
 * ******************************************************************************************/
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





/* ******************************************************************************************
 * **
 * **  Constructor
 * **
 * ******************************************************************************************/

/**
 * Construct a field given a point cloud
 */
Field::Field( const pcl::PointCloud<pcl::PointNormal>::Ptr cloud, int k, bool tracing_enabled ) {
	m_tracing_enabled = tracing_enabled;

	// First make an empty FieldGraph
	m_graph = new FieldGraph( );

	// Next add all the points to it
	std::map<pcl::PointNormal, FieldGraphNode*, cmpPointNormal> point_to_gn_map;
	size_t points_added = 0;
    for( auto point : *cloud ) {

		FieldElement * fe = FieldElement::from_point( point );
		FieldGraphNode * gn = m_graph->add_node( fe );

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
            	    m_graph->add_edge( this_gn, neighbour_gn, 1.0f, nullptr );
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
}

/**
 * Destructor for fields
 */
Field::~Field( ) {
	delete m_graph;
}

/**
 * @return the size of the ifled
 */
std::size_t Field::size() const {
	return m_graph->num_nodes();
}


/**
 * Return vector of elements
 */
const std::vector<const FieldElement *> Field::elements( ) const {
	std::vector<const FieldElement *> elements;

	for( auto node : m_graph->nodes() ) {
		elements.push_back( node->data() );
	}
	return elements;
}
}