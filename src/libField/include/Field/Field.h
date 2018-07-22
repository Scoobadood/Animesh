#pragma once 

#include <Graph/Graph.h>
#include <Graph/GraphSimplifier.h>
#include <Eigen/Core>
#include <pcl/point_types.h>
#include <pcl/point_cloud.h>

#include <Field/FieldElement.h>

namespace animesh {

/**
 * A Field is the collection of FieldElements and their relationships
 * including spatial neghbours and node correspondences across time
 */

class Field {
	using FieldGraph = Graph<FieldElement *, void *>;
	using FieldGraphNode = typename Graph<FieldElement *, void *>::GraphNode;

	/* ******************************************************************************************
	 * *
	 * *  Building Fields
	 * * 
	 * ******************************************************************************************/

public:
	/**
	 * Construct from a PCL PointCloud
	 */
	Field( const pcl::PointCloud<pcl::PointNormal>::Ptr cloud, int k, bool tracing_enabled = false );

	Field( const std::string file_name, int k, bool tracing_enabled = false );

	~Field( );

	/* ******************************************************************************************
	 * *
	 * *  Attributes
	 * * 
	 * ******************************************************************************************/
public:
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
	 * *  IO
	 * * 
	 * ******************************************************************************************/
	void dump( ) const;

	void enable_tracing( bool enable_tracing ) { m_tracing_enabled = enable_tracing;}

	friend std::ostream& operator<<( std::ostream&, const FieldElement&);


public:
		/** The graph of the nodes */
	FieldGraph *			m_graph;

private:
	void trace_vector( const std::string& prefix, const Eigen::Vector3f& vector ) const;
	void trace_node( const std::string& prefix, const FieldElement * this_fe ) const;

	/** Flag to determine if we should trace field moothing */
	bool 					m_tracing_enabled;

};

std::ostream& operator<<( std::ostream& os, const Eigen::Vector3f& fe);
}