#pragma once 

#include <Graph/Graph.h>
#include <Graph/GraphBuilder.h>
#include <pcl/point_types.h>
#include <pcl/kdtree/kdtree_flann.h>

class Field {
public:
	/**
	 * Construct the field using a graph builder and some elements
	 */
	Field( const GraphBuilder<void *> * const graph_builder, const std::vector<Element>& elements );

	/**
	 * Construct from a PCL PointCloud
	 */
	Field( const pcl::PointCloud<pcl::PointXYZ>::Ptr cloud, int k );

	~Field( );

	/**
	 * @return the size of the ifled
	 */
	std::size_t size() const;

	/**
	 * Construct a planar field centred at (0,0,0)
	 * @param dim_x The number of points in the X plane
	 * @param dim_y The number of points in the Y plane
	 * @param grid_spacing The space between grid points
	 * @param make_fixed If true, set the field tangents to the lowest energy/solved position
	 */
	static Field * planar_field( std::size_t dim_x, std::size_t dim_y, float grid_spacing, bool make_fixed );

	/**
	 * Construct a curved field centred at (0,0,0). THe field is defined by z=1-\frac{1}{10}\left( x^{2}+y^{2} \right)
	 * with grid neighbourhod
	 * @param dim_x The number of points in the X plane
	 * @param dim_y The number of points in the Y plane
	 * @param grid_spacing The space between grid points
	 * @param make_fixed If true, set the field tangents to the lowest energy/solved position
	 */
	static Field * polynomial_field( std::size_t dim_x, std::size_t dim_y, float grid_spacing, bool make_fixed );

	/**
	 * Construct a spherical field centred at (0,0,0)
	 * @param radius The radius of the sphere to be constructed
	 * @param theta_steps The number of steps around the sphere (in XZ plane)
	 * @param phi_steps The number of steps in the Y direction
	 * @param make_fixed If true, set the field tangents to the lowest energy/solved position
	 */
	static Field * spherical_field( float radius, std::size_t theta_steps, std::size_t phi_steps, int k, bool make_fixed);
	static Field * triangular_field( float tri_radius);
	static Field * cubic_field(std::size_t cube_size, bool make_fixed);
	static Field * circular_field( float radius, int k, bool make_fixed );

	/**
	 * Smooth the field once, applying smoothing to each node
	 * @return the largest error in tangent
	 */
	void smooth_once( );

	/**
	 * Smooth the specified node (and neighbours)
	 * @return The new vector.
	 */
	void smooth_node( const GraphNode<FieldElement *, void*> * const gn ) const;

	/**
	 * @return vector of elements 
	 */
	const std::vector<const FieldElement *> elements( ) const;

	/**
	 * Set all the tangents in the field to specific values.
	 * @param new_tangents The new tangents
	 * @return The difference between the old and new values
	 */
	void set_tangents( const std::vector<const Eigen::Vector3f>& new_tangents );

	void dump( ) const;

	void enable_tracing( bool enable_tracing ) { m_tracing_enabled = enable_tracing;}

	/**
	 * @return the smoothness
	 */
	float error( ) const;

private:
	/** The Graph - helps us get neighbours */
	Graph<FieldElement *, void*> *  	m_graph;

	/** Flag to determine if we should trace field moothing */
	bool 		m_tracing_enabled;

	void trace_vector( const std::string& prefix, const Eigen::Vector3f& vector ) const;
	void trace_node( const std::string& prefix, const FieldElement * this_fe ) const;
	void init( const GraphBuilder<void*> * const graph_builder, const std::vector<Element>& elements );



	/**
 	 * @return the smoothness of one node
	 */
	float get_error_for_node( const GraphNode<FieldElement *, void*> * gn ) const;
};