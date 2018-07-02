#pragma once 

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
	Field( const pcl::PointCloud<pcl::PointNormal>::Ptr cloud, int k, bool tracing_enabled = false );

	~Field( );

	/**
	 * Construct prefab fields 
	 */
	static Field * planar_field( std::size_t dim_x, std::size_t dim_y, float grid_spacing, bool make_fixed );
	static Field * spherical_field( float radius, std::size_t theta_steps, std::size_t phi_steps, int k, bool make_fixed);
	static Field * triangular_field( float tri_radius);
	static Field * cubic_field(std::size_t cube_size, float scale, bool make_fixed);
	static Field * circular_field( float radius, int k, bool make_fixed );
	static Field * polynomial_field( std::size_t dim_x, std::size_t dim_y, float grid_spacing, bool make_fixed );



	/**
	 * @return the size of the ifled
	 */
	std::size_t size() const;

	/**
	 * @return vector of elements 
	 */
	const std::vector<const FieldElement *> elements( ) const;

	/**
	 * Smooth the field
	 */
	void smooth( );

	/**
	* Smooth the field
	*/
	void smooth_completely();

	/**
	 * @return the smoothness of the entire Field
	 */
	float calculate_error( animesh::Graph<FieldElement *, void*> * tier ) const;

	/**
	 * Set all the tangents in the field to specific values.
	 * @param new_tangents The new tangents
	 * @return The difference between the old and new values
	 */
	void set_tangents( const std::vector<const Eigen::Vector3f>& new_tangents );

	void dump( ) const;

	void enable_tracing( bool enable_tracing ) { m_tracing_enabled = enable_tracing;}

	friend std::ostream& operator<<( std::ostream&, const FieldElement&);

private:
	// Smoothing
	/**
	 * Smooth the current tier of the hierarchy by repeatedly smoothing until the error doesn't change
	 * significantly.
	 * @return true if the tier has converged
	 */
	bool smooth_tier( animesh::Graph<FieldElement *, void*> * tier );

	/**
	 * Smooth the field once, applying smoothing to each node
	 * @return the total residual
	 */
	float smooth_once( animesh::Graph<FieldElement *, void*> * tier );

	/**
	 * Smooth the specified node (and neighbours)
	 * @return The new vector.
	 */
	Eigen::Vector3f calculate_smoothed_node( animesh::Graph<FieldElement *, void*> * tier, 
								 			 animesh::Graph<FieldElement *, void*>::GraphNode * const gn ) const;
	/**
	 * @return the smoothness of one node
	 */
	float error_for_node( animesh::Graph<FieldElement *, void*> * tier, 
						  animesh::Graph<FieldElement *, void*>::GraphNode * const gn ) const;

	void randomise_tangents( );
	/**
	 * Generate the hierarchical grah by repeatedly simplifying until there are e.g. less than 20 nodes
	 */
	void generate_hierarchy( size_t max_nodes );
	void trace_vector( const std::string& prefix, const Eigen::Vector3f& vector ) const;
	void trace_node( const std::string& prefix, const FieldElement * this_fe ) const;
	void init( const GraphBuilder<void*> * const graph_builder, const std::vector<Element>& elements );
	float calculate_error_for_node( animesh::Graph<FieldElement *, void*> * tier, 
		animesh::Graph<FieldElement *, void*>::GraphNode * gn ) const;

	/** The Graph - helps us get neighbours */
	animesh::Graph<FieldElement *, void*> *  	m_graph;

	/** The top of hierarchy Graph */
	animesh::Graph<FieldElement *, void*> *  	m_top_graph;

	/** Flag to determine if we should trace field moothing */
	bool 		m_tracing_enabled;

	/** Smoothing in progress */
	bool									m_smoothing;
	bool									m_new_tier;
	float									m_last_error;
	int 									m_tier_index;
	animesh::Graph<FieldElement *, void*> *	m_current_tier;
};

std::ostream& operator<<( std::ostream& os, const Eigen::Vector3f& fe);
