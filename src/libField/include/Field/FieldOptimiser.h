#include <Field/Field.h>
#include <Graph/GraphSimplifier.h>
#include <Field/CorrespondenceMapping.h>

namespace animesh {

using FieldGraph            = Graph<FieldElement *, void *>;
using FieldGraphNode        = typename Graph<FieldElement *, void *>::GraphNode;
using FieldGraphSimplifier    = GraphSimplifier<FieldElement *, void *>;
using FieldGraphMapping    = GraphSimplifier<FieldElement *, void *>::GraphMapping;

class FieldOptimiser {

public:
    /**
     * Construct with a Field to be optimised
     */
    FieldOptimiser(Field *field);

    /**
     * Optimize the field in one go
     */
    void optimise();

    /**
     * Perform one step of omptimisation
     */
    void optimise_one_step();

    inline int num_tiers() const { return m_graph_hierarchy.size(); }

    /**
     * Current error in field
     */
    float current_error(int tier) const;

    /**
     * @Return the nth graph in the hierarchy where 0 is base.
     */
    FieldGraph *graph_at_tier(size_t tier) const;

    /**
     * @Return the current tier being optimised or 0 if none
     */
    size_t optimising_tier_index() const;

    /**
     * @return the FE corresponding to the given one in a given tier and frame
     */
    const FieldElement*
    get_corresponding_fe_in_frame( size_t frame_idx, size_t tier_idx, const FieldElement* src_fe  ) const;

    std::vector<FieldElement*> 
    get_elements_at( size_t frame_idx, size_t tier_idx ) const;
    
private:

    /**
     */
    void update_tangents( size_t tier_idx, const std::vector<Eigen::Vector3f> new_tangents );


    /**
     * Start optimising.
     */
    void setup_optimisation();

    /**
     * For each tier of the graph hierarchy, build an equivalent set of correspondences
     * to the newly generated FEs
     */
    void build_correspondences();

    /**
     * Mark optimisation as done.
     */
    void stop_optimising();

    /**
     * Start a brand ew optimisation session
     */
    void setup_tier_optimisation();

    /**
     * Smooth the current tier of the hierarchy once and return true if it converged
     * @param tier The Graph (tier) to be optimised
     */
    bool optimise_tier_once(FieldGraph *tier);

    /**
     * @return true if the optimisation operation has converged
     * (or has iterated enough times)
     * otherwise return false
     */
    bool check_convergence(float new_error);

    /**
     * Smooth the specified node
     * @return The new vector.
     */
    Eigen::Vector3f calculate_smoothed_node(FieldGraph *tier, FieldGraphNode *gn) const;

    /**
     * @return the smoothness of the entire Field
     */
    float calculate_error(FieldGraph *tier) const;

    /**
     * @return the smoothness of one node
     */
    float calculate_error_for_node(FieldGraph *tier, FieldGraphNode *gn) const;

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
    void generate_hierarchy(int max_tiers, int max_nodes, int max_edges);

    void build_equivalent_fes( );
    void build_transforms( );

    size_t index(size_t frame_idx, size_t tier_idx) const;

    std::vector<FieldElement*> get_corresponding_fes_in_frame(size_t frame_idx, size_t tier_idx, std::vector<FieldElement*> fes) const;

    Field *                             m_field;

    /** A hierarchy of graphs **/
    std::vector<FieldGraph *>           m_graph_hierarchy;
    std::vector<FieldGraphMapping>      m_mapping_hierarchy;

    std::vector<FieldElement*>   *      m_field_element_mappings;
    std::vector<Eigen::Matrix3f> *      m_transforms;


    /** Flag to determine if we should trace field moothing */
    bool m_tracing_enabled;

    /** Smoothing in progress */
    bool m_is_optimising;
    bool m_optimising_started_new_tier;
    float m_optimising_last_error;
    int m_optimising_iterations_this_tier;
    int m_optimising_tier_index;
    FieldGraph *m_optimising_current_tier;
};
}