#pragma once 

#include <Graph/Graph.h>
#include <Eigen/Core>
#include <pcl/point_types.h>
#include <pcl/point_cloud.h>

#include <Field/FieldElement.h>

namespace animesh {

using FieldGraph = Graph<FieldElement *, void *>;
using FieldGraphNode = typename Graph<FieldElement *, void *>::GraphNode;

// Fwd decl For point comparison
struct cmpPointNormal;

/**
 * A Field is the collection of FieldElements across multuiple timepoints
 * The spatial relationship is modeled in a Graph and the temporal relationship
 * is modeled in a series of maps.
 */
class Field {

public:
	/**
	 * Construct from a PCL PointCloud
	 */
	Field( const pcl::PointCloud<pcl::PointNormal>::Ptr cloud, int k = 5, bool tracing_enabled = false );

	/**
	 * Construct from multiple PCL PointClouds
	 */
	// Field( const std::vector<pcl::PointCloud<pcl::PointNormal>::Ptr> clouds, int k = 5, bool tracing_enabled = false );

	Field( const std::string file_name, int k, bool tracing_enabled = false );

	~Field( );

	/**
	 * @return the size of the field (number of FieldElements)
	 */
	std::size_t size() const;

	/**
	 * @return vector of elements 
	 */
	const std::vector<const FieldElement *> elements( ) const;


	/* ******************************************************************************************
	 * *
	 * *  Multi Frame Support
	 * * 
	 * ******************************************************************************************/

	/**
	 * Add a point cloud for the next point in time.
	 * @param cloud The point cloud to be added for the next time point.
	 */ 
	void add_new_timepoint( const pcl::PointCloud<pcl::PointNormal>::Ptr cloud );


	/**
	 * Get the vector elements neighbouring the given node at the specific time point.
	 * @param fe The FieldElement who's neighbours should be returned
	 * @param time_point The neighbouring FieldElements
	 * @return vector of elements 
	 */
	std::vector<FieldElement *> get_neighbours_of( FieldElement * fe, int time_point ) const;

	/**
	 * Get the point corresponding to this one at a given time point
	 * @param fe The source FieldElement
	 * @param time_point The time point from which to recover the corresponding point
	 * @return The corresponding element
	 */
	FieldElement * const get_point_corresponding_to( const FieldElement * const fe, int time_point ) const;

	/**
	 * Get the Matrix which transforms a given FE into it's position at time t
	 * @param fe The FieldElement to map
	 * @param t The timepoint
	 * @return The mtransformation matrix 
	 */
	Eigen::Matrix3f get_fwd_xform_for( const FieldElement * const node, int time_point ) const;


	std::vector<FieldElement *> get_all_n_at_t0() const;

	size_t get_num_timepoints() const;


	/* ******************************************************************************************
	 * *
	 * *  IO
	 * * 
	 * ******************************************************************************************/
	void dump( ) const;

	void enable_tracing( bool enable_tracing ) { m_tracing_enabled = enable_tracing;}

	/** Vector of maps from first frame FieldElement to Point/xform at time t */
	using FutureFieldElementAndRotation = std::pair<FieldElement *, Eigen::Matrix3f>;
	using Correspondence = std::map<FieldElement *, std::pair<FieldElement *, Eigen::Matrix3f>>;
	std::vector<Correspondence>	m_correspondences;

	/** The graph of the nodes constructed over the first pointcloud */
	FieldGraph *				m_graph;

private:
	/**
	 * Used in the cinstruction of a Field. Adds the points and builds a map from
	 * points to GraphNode to facilitate nearest neighbour calculations.
	 * @param cloud The point cloud to add.
	 * @return the mapping
	 */
	std::map<pcl::PointNormal, FieldGraphNode*, cmpPointNormal> * 
	add_points_from_cloud( const pcl::PointCloud<pcl::PointNormal>::Ptr cloud);


	/**
	 *  Throw an exception if the time_point is out of range
	 *
	 * @param time_point
	 */
	void inline check_time_point( int time_point ) const {
        if (time_point < 1 || time_point >= m_correspondences.size())
            throw std::invalid_argument("Time point out of range");
    }

    /** Flag to determine if we should trace field moothing */
	bool 											m_tracing_enabled;

	/** Maintain a list of elements in the first time point */
	std::vector<FieldElement *> 					m_elements;

	/** Maintain a mapping between FieldElements and GraphNodes */
	std::map<FieldElement *, FieldGraphNode *>		m_graphnode_for_field_element;
};
/* ******************************************************************************************
 * *
 * *  Utility Functions
 * * 
 * ******************************************************************************************/

/**
 * Get the point corresponding to this one at a given time point
 * @param fe The source FieldElement
 * @param time_point The time point from which to recover the corresponding point
 * @return The corresponding element
 */
void compute_temporal_transform( FieldElement * node_at_t0, 
	const std::vector<FieldElement *>& neighbours_at_0, 
	Field::Correspondence& corr );

/**
 * Find correspondences between FieldElements in first set and second set.
 */
void find_correspondences( 
	const std::vector<FieldElement * >& first, 
	const std::vector<FieldElement * >& second, 
	Field::Correspondence& corr );

std::ostream& operator<<( std::ostream& os, const Eigen::Vector3f& fe);
}