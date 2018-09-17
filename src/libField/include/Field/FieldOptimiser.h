#include <Field/PointNormal.h>
#include <Graph/GraphSimplifier.h>

namespace animesh {

class FieldOptimiser {
    /* ******************************************************************************************
     *
     * New representation data storage
     *
     * ******************************************************************************************/
    using PointNormalGraph    = Graph<PointNormal::Ptr, void *>;
    using PointNormalGraphPtr = std::shared_ptr<PointNormalGraph>;
    using PointNormalGraphSimplifier = GraphSimplifier<PointNormal::Ptr, void *>;
    using PointNormalGraphMapping = GraphSimplifier<PointNormal::Ptr, void *>::GraphMapping;

    std::vector< /* tiers */
    std::vector< /* frames */
    std::vector< /* vertices */PointNormal::Ptr>>>      m_tiers;
    std::vector<Eigen::Vector3f>                        m_tangents;
    std::multimap<size_t, size_t>                       m_adjacency;
    std::vector<PointNormalGraphPtr>                    m_graphs;
    std::vector<PointNormalGraphMapping>                m_mappings;
    std::vector</* tiers */
    std::vector</* frames */
    std::vector</* vertices */
    std::pair<Eigen::Matrix3f, Eigen::Matrix3f>>>>      m_point_transforms;
    bool                                                m_is_optimising;
    bool                                                m_optimising_started_new_tier;
    float                                               m_optimising_last_error;
    int                                                 m_optimising_iterations_this_tier;
    size_t                                              m_optimising_tier_idx;
    PointNormalGraphPtr                                 m_optimising_current_tier;
    std::vector<bool>                                   m_include_frames;
    bool                                                m_tracing_enabled;
    std::unordered_set<Path>                            m_cycles;

    /* ******************************************************************************************
     *
     *   New public methods
     *
     * ******************************************************************************************/
public:
    /**
     * Build a FieldOptimiser to optimise the given data.
     * Data consists of a number of frames as well as adjacency data.
     * The frame data is assumed to be in correspondence.
     *
     * @param frames The frames of data.
     * @param adjacency A mape describing adjacency. Indices in the map correspond to the point order in frames.
     */
    FieldOptimiser(const std::vector<std::vector<PointNormal::Ptr>>& frames, const std::multimap<std::size_t, std::size_t>& adjacency);

    /**
     * Return the vertex data for a specific tier and frame
     */
    const std::vector<PointNormal::Ptr>&
    point_normals_for_tier_and_frame( std::size_t tier_idx, std::size_t frame_idx ) const;

    /**
     * Return the specific vertex for a tier, frame and vertex index
     */
    const PointNormal::Ptr&
    point_normal_for_tier_and_frame( std::size_t tier_idx, std::size_t frame_idx, std::size_t vertex_idx ) const;

    std::vector<Eigen::Vector3f>
    propagate_tangents_up( const std::vector<Eigen::Vector3f>& tangents, std::size_t tier_idx ) const;

    std::vector<Eigen::Vector3f>
    propagate_tangents_down( const std::vector<Eigen::Vector3f>& tangents, std::size_t tier_idx ) const;

    /**
     * Reproject the tangents from frame 0 tier 0 into an arbitrary frame and tier by using the forward transformations
     * and then reprojecting into tangent plane and normalising.
     * @return The tangents as in tier and frame.
     */
    std::vector<Eigen::Vector3f>
    compute_tangents_for_tier_and_frame(size_t tier_idx, size_t frame_idx) const;

    /**
     * @return The current error. This is calculated on the current tier when smoothing and tier0 when not.
     */
    float
    total_error() const;

    /**
     * Run the optimiser to completion.
     */
    void optimise();

    /**
     * Perform a single step of optimisation.
     */
    void
    optimise_do_one_step();

    /**
     * @return the optimising index.
     */
    size_t
    optimising_tier_index( ) const;

    /**
     * @return true if the optimiser is running.
     */
    bool
    is_optimising( ) const;

    /**
     * Run the optimiser to completion.
     */
    void randomise();

    /**
     * @return The number of tiers in the graph hierarchy.
     */
    std::size_t
    num_tiers() const;

    /**
     * @return The number of frames
     */
    std::size_t
    num_frames() const;

    /**
     * Include or exclude the frame from smoothing.
     */
    void
    enable_frame(size_t frame_idx, bool enable);

    /**
     * @return true if a frame is included in smoothing, else false.
     */
    bool
    is_frame_enabled(size_t frame_idx) const;

    /**
     * @return the numberof nodes in the given tier.
     */
    std::size_t
    num_nodes_in_tier( std::size_t tier_idx) const;

    /**
     * @return The number of edges in the given tier.
     */
    std::size_t
    num_edges_in_tier( std::size_t tier_idx) const;

    /**
     * @return true if the frame is to be included in the optimisation.
     */
    bool
    should_include_frame(std::size_t frame_idx) const;

    /**
     * @return a vector of vectors of indices for the neighbours of each point in a tier.
     */
    std::vector<std::vector<std::size_t>>
    adjacency_for_tier(std::size_t tier_idx) const;

    /**
     * @return the mean edge length in the graph
     */
    float
    mean_edge_length_for_tier(std::size_t tier_idx ) const;


    /* ******************************************************************************************
     *
     *   New private methods
     *
     * ******************************************************************************************/
private:
    /**
     * Initialise the FieldOptimiser given a set of inital frame data and adjacency information
     */
    void initialise();

    /**
     * @return The transformation matrix for a specific vertex from frame0 in tier_idx to frame_idx.
     */
    const Eigen::Matrix3f&
    forward_transform_to( size_t tier_idx, size_t frame_idx, size_t node_idx ) const;

    /**
     * Computes the new tangent for a given node by averaging over all neighbours. Final result is
     * projected back into tangent space for the given FE's normal
     * @return The new vector.
     */
    Eigen::Vector3f
    compute_new_tangent_for_vertex(const std::vector<PointNormalGraphPtr>& graphs, size_t tier_idx, size_t vertex_idx) const;

    /**
     * Construct a vector of all neighbours of a given graph node in a specific tier.
     * This method uses the graph to extract immediate neighbours at frame 0 in this tier
     * and then identifies corresponding FieldElements in other frames and back-projects them to
     * frame 0 using the inverse transformation matrix constructed during initialisation.
     *
     * @param tier_idx The tier of the graph hierarchy to consider.
     * @param vertex_idx The vertex within a frame for which we're calculating this.
     * @return A pair of vectors, the first are normals and the second tangents for all neighbours of this vertex.
     */
    std::pair<std::vector<Eigen::Vector3f>, std::vector<Eigen::Vector3f>>
    copy_all_neighbours_for(const std::vector<PointNormalGraphPtr>& graphs, std::size_t tier_idx, std::size_t vertex_idx) const;


    /**
     * Compute new tangents for all vertices in a tier
     * @param tier_idx The index of the graph tier to be optimised.
     * @return True if the optimisation converged, otherwise false.
     */
    std::vector<Eigen::Vector3f>
    compute_new_tangents_for_tier(const std::vector<PointNormalGraphPtr>& graphs, std::size_t tier_idx) const;

    /**
     * Update the tangents (for currently optimising tier).  The provided vector of tangents is in
     * order specified by indices.
     */
    void
    update_tangents( const std::vector<Eigen::Vector3f>& new_tangents);

    /**
     * Setup for optimisation. Build the hierarchical graph and
     * construct correspondence maps.
     */
    void
    optimise_begin();

    /**
     * Start optimising a brand new tier of the hierarchical graph,
     */
    void
    optimise_begin_tier();

    /**
     * Handle end of optimising for a tier.
     */
    bool
    optimise_end_tier();

    /**
     * Optimisation has concluded.
     */
    void
    optimise_end();

    /**
     * @return true if the optimisation operation has converged
     * (or has iterated enough times)
     * otherwise return false
     */
    bool check_convergence(float new_error) const;
};
}
