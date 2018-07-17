#pragma once 

#include <Graph/Graph.h>
#include <Graph/GraphSimplifier.h>
#include <pcl/point_types.h>
#include <pcl/kdtree/kdtree_flann.h>

struct FieldElement {
	Eigen::Vector3f		m_location;
	Eigen::Vector3f		m_normal;
	Eigen::Vector3f		m_tangent;

	FieldElement( Eigen::Vector3f location, 
				  Eigen::Vector3f normal,
				  Eigen::Vector3f tangent ) : m_location{ location }, m_normal{ normal }, m_tangent{ tangent } {};
	/**
 	 * Useful method for merging FieldElements
	 */
	static FieldElement * mergeFieldElements ( const FieldElement * const fe1, const FieldElement * const fe2 );

	/**
 	 * Propagate field element changes down graph hierarch
	 */
	static FieldElement * propagateFieldElements ( const FieldElement * const parent, const FieldElement * const child );
};


class Field {
	using FieldGraph = typename animesh::Graph<FieldElement *, void *>;
	using FieldGraphNode = typename animesh::Graph<FieldElement *, void *>::GraphNode;
	using FieldGraphSimplifier = typename animesh::GraphSimplifier<FieldElement *, void *>;
	using FieldGraphMapping = typename animesh::GraphSimplifier<FieldElement *, void *>::GraphMapping;

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

	/**
	 * Construct prefab fields 
	 */
	static Field * planar_field( std::size_t dim_x, std::size_t dim_y, float grid_spacing, int k );
	static Field * cubic_field( int cube_x, int cube_y, int cube_z, float scale, int k);
	static Field * polynomial_field( std::size_t dim_x, std::size_t dim_y, float grid_spacing, int k);
	static Field * spherical_field( float radius, std::size_t theta_steps, std::size_t phi_steps, int k);
	static Field * circular_field( float radius, int k );

	void randomise_tangents( );

	/**
	 * Generate a hierarchical graph by repeatedly simplifying until there are e.g. less than 20 nodes
	 * Stash the graphs and mappings into vectors.
	 * Tries to respect the parameters provided. If multiple paramters are provided it will terminate at
	 * the earliest.
	 * @param max_edges >0 means keep iterating until only this number of edges remain. 0 means don't care.
	 * @param max_nodes >0 means keep iterating until only this number of nodes remain. 0 means don't care.
	 * @param max_tiers >0 means keep iterating until only this number of tiers exist. 0 means don't care.
	 * 
	 */
	void generate_hierarchy( int max_tiers, int max_nodes, int max_edges );


	/**
	 * Clear all variables. Called prior to loading new object and on termination
	 */
	void clear_up( );


	/* ******************************************************************************************
	 * *
	 * *  Smoothing Fields
	 * * 
	 * ******************************************************************************************/
public:
	/**
	 * Smooth the field (once)
	 */
	void smooth( );

	/**
	* Smooth the field completelt
	*/
	void smooth_completely();

private:

	/**
	 * Start a brand new smoothing session
	 */
	void start_smoothing( );

	/**
	 * Start a brand ew smoothing session
	 */
	void start_smoothing_tier( );
	/**
	 * Smooth the current tier of the hierarchy by repeatedly smoothing until the error doesn't change
	 * significantly.
	 * @return true if the tier has converged
	 */
	bool smooth_tier_once( FieldGraph * tier );

	/**
	 * @return true if the smoothing operation has converged
	 * (or has iterated enough times)
	 * otherwise return false
	 */
	bool check_convergence( float new_error );

	/**
	 * Smooth the specified node (and neighbours)
	 * @return The new vector.
	 */
	Eigen::Vector3f calculate_smoothed_node( FieldGraph * tier, FieldGraphNode * const gn ) const;


	/* ******************************************************************************************
	 * *
	 * *  Resdiduals
	 * * 
	 * ******************************************************************************************/
public:

	/**
	 * Current error in field
	 */
	float current_error( int tier ) const;

private:

	/**
	 * @return the smoothness of one node
	 */
	float error_for_node( FieldGraph * tier, FieldGraphNode * const gn ) const;

	float calculate_error_for_node( FieldGraph * tier, FieldGraphNode * gn ) const;

	/**
	 * @return the smoothness of the entire Field
	 */
	float calculate_error( FieldGraph * tier ) const;


	/* ******************************************************************************************
	 * *
	 * *  Attributes
	 * * 
	 * ******************************************************************************************/
public:
	/**
	 * @return the size of the ifled
	 */
	std::size_t size() const;

	/**
	 * @param tier The tier for which the elements should be returned
	 * @return vector of elements 
	 */
	const std::vector<const FieldElement *> elements( int tier ) const;

	inline int num_tiers( ) const { return m_graph_hierarchy.size(); }

	/**
	 * @Return the nth graph in the hierarchy where 0 is base.
	 */
	FieldGraph * graph_at_tier( size_t tier ) const;

	/* ******************************************************************************************
	 * *
	 * *  IO
	 * * 
	 * ******************************************************************************************/
	void dump( ) const;

	void enable_tracing( bool enable_tracing ) { m_tracing_enabled = enable_tracing;}

	friend std::ostream& operator<<( std::ostream&, const FieldElement&);

private:
	void trace_vector( const std::string& prefix, const Eigen::Vector3f& vector ) const;
	void trace_node( const std::string& prefix, const FieldElement * this_fe ) const;

	/** A hierarchy of graphs **/
	std::vector<FieldGraph *>		m_graph_hierarchy;
	std::vector<FieldGraphMapping>	m_mapping_hierarchy;

	/** Flag to determine if we should trace field moothing */
	bool 									m_tracing_enabled;

	Field * m_field;
	/** Smoothing in progress */
	bool									m_is_smoothing;
	bool									m_smoothing_started_new_tier;
	float									m_smoothing_last_error;
	int 									m_smoothing_iterations_this_tier;
	int 									m_smoothing_tier_index;
	animesh::Graph<FieldElement *, void*> *	m_smoothing_current_tier;
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
