#pragma once 

#include <Graph/Graph.h>
#include <Graph/GraphSimplifier.h>
#include <pcl/point_types.h>
#include <pcl/kdtree/kdtree_flann.h>

namespace animesh {


/**
 * FieldElement stores the location, normal and tangent for each
 * element of the directional field
 */
struct FieldElement {
	Eigen::Vector3f		m_location;
	Eigen::Vector3f		m_normal;
	Eigen::Vector3f		m_tangent;

	/**
	 * Construct with location normal and tangent
	 */
	FieldElement( Eigen::Vector3f location,  Eigen::Vector3f normal, Eigen::Vector3f tangent ) : m_location{ location }, m_normal{ normal }, m_tangent{ tangent } {};

	/**
 	 * Useful method for merging FieldElements
	 */
	static FieldElement * mergeFieldElements ( const FieldElement * const fe1, const FieldElement * const fe2 );

	/**
 	 * Propagate field element changes down graph hierarch
	 */
	static FieldElement * propagateFieldElements ( const FieldElement * const parent, const FieldElement * const child );
};


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

	void randomise_tangents( );

	/* ******************************************************************************************
	 * *
	 * *  Construct prefab fields
	 * * 
	 * ******************************************************************************************/
	static Field * planar_field( std::size_t dim_x, std::size_t dim_y, float grid_spacing, int k );
	static Field * cubic_field( int cube_x, int cube_y, int cube_z, float scale, int k);
	static Field * polynomial_field( std::size_t dim_x, std::size_t dim_y, float grid_spacing, int k);
	static Field * spherical_field( float radius, std::size_t theta_steps, std::size_t phi_steps, int k);
	static Field * circular_field( float radius, int k );

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

/**
 * Load an obj file into a point cloud
 */
pcl::PointCloud<pcl::PointNormal>::Ptr load_pointcloud_from_obj( const std::string& file_name );

/**
 * Construct a field from an OBJ file
 */
Field * load_field_from_obj_file( const std::string& file_name, int k = 5, float with_scaling = 1.0f, bool trace = false );
}