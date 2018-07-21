#include <Field/Field.h>
#include <Graph/GraphSimplifier.h>

namespace animesh {

using FieldGraph = Graph<FieldElement *, void *>;
using FieldGraphNode = typename Graph<FieldElement *, void *>::GraphNode;
using FieldGraphSimplifier = GraphSimplifier<FieldElement *, void *>;
using FieldGraphMapping = GraphSimplifier<FieldElement *, void *>::GraphMapping;

class FieldOptimiser {

public:
	/**
	 * Construct with a Field to be optimised
	 */
	FieldOptimiser( Field* field );

	/**
	 * Optimize the field in one go
	 */
	void optimise( );

	/**
	 * Perform one step of omptimisation
	 */
	void optimise_once( );

	inline int num_tiers( ) const { return m_graph_hierarchy.size(); }

	/**
	 * Current error in field
	 */
	float current_error( int tier ) const;


	/**
	 * @Return the nth graph in the hierarchy where 0 is base.
	 */
	FieldGraph * graph_at_tier( size_t tier ) const;

	/**
	 * @Return the current tier being optimised or 0 if none
	 */
	size_t optimising_tier_index( ) const;

private:
	/**
	 * Start optimising.
	 */
	void start_optimising( );

	/**
	 * Mark optimisation as done.
	 */
	void stop_optimising();

	/**
	 * Start a brand ew optimisation session
	 */
	void start_optimising_tier( );

	/**
	 * Smooth the current tier of the hierarchy once and return true if it converged
	 * @param tier The Graph (tier) to be optimised
	 */
	bool optimise_tier_once ( FieldGraph * tier );

	/**
	 * @return true if the optimisation operation has converged
	 * (or has iterated enough times)
	 * otherwise return false
	 */
	bool check_convergence( float new_error );

	/**
	 * Smooth the specified node
	 * @return The new vector.
	 */
	Eigen::Vector3f calculate_smoothed_node( FieldGraph * tier, FieldGraphNode * gn ) const;

	/**
	 * @return the smoothness of the entire Field
	 */
	float calculate_error( FieldGraph * tier ) const;

	/**
	 * @return the smoothness of one node
	 */
	float calculate_error_for_node( FieldGraph * tier, FieldGraphNode * gn ) const;

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

	Field *    								m_field;

	/** A hierarchy of graphs **/
	std::vector<FieldGraph *>				m_graph_hierarchy;
	std::vector<FieldGraphMapping>			m_mapping_hierarchy;

	/** Flag to determine if we should trace field moothing */
	bool 									m_tracing_enabled;

	/** Smoothing in progress */
	bool									m_is_optimising;
	bool									m_optimising_started_new_tier;
	float									m_optimising_last_error;
	int 									m_optimising_iterations_this_tier;
	int 									m_optimising_tier_index;
	animesh::Graph<FieldElement *, void*> *	m_optimising_current_tier;
};
}