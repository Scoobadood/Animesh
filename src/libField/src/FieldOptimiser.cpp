#include <Field/FieldOptimiser.h>
#include <Field/Checks.h>
#include <Field/PointNormal.h>

#include <RoSy/RoSy.h>
#include <Eigen/Geometry>

#define throw_invalid_argument(msg) \
    throw std::invalid_argument(msg " at " __FILE__ ":" + std::to_string(__LINE__))
#define throw_runtime_error(msg) \
    throw std::runtime_error(msg " at " __FILE__ ":" + std::to_string(__LINE__))

const float EPSILON = 1e-4;
const int MAX_ITERS_PER_TIER = 20;
const float CONVERGENCE_THRESHOLD = 0.5f;
const int SMOOTH_EDGES = 10;
const int SMOOTH_NODES = 20;
const int SMOOTH_TIERS = 10;

using animesh::FieldOptimiser;

/* ******************************************************************************************
 *
 *   New
 *
 * ******************************************************************************************/
using animesh::Graph;
using animesh::GraphSimplifier;
using animesh::PointNormal;
using PointNormalGraph    = Graph<PointNormal::Ptr, void *>;
using PointNormalGraphNode = Graph<PointNormal::Ptr, void *>::GraphNode;
using PointNormalGraphPtr = std::shared_ptr<PointNormalGraph>;
using PointNormalGraphSimplifier = GraphSimplifier<PointNormal::Ptr, void *>;
using PointNormalGraphMapping = GraphSimplifier<PointNormal::Ptr, void *>::GraphMapping;

/**
 * @return the error of a single vertex in a tier
 */
float
compute_error_for_vertex(
    size_t vertex_idx,
    std::vector<size_t> neighbour_indices,
    const std::vector<PointNormal::Ptr>& vertices,
    const std::vector<Eigen::Vector3f>& tangents ) {
    using namespace std;
    using namespace Eigen;

    float error = 0.0f;

    Vector3f this_normal = vertices[vertex_idx]->normal();
    Vector3f this_tangent = tangents[vertex_idx];
    for (auto other_idx : neighbour_indices) {
        Vector3f other_normal = vertices[other_idx]->normal();
        Vector3f other_tangent = tangents[other_idx];
        pair<Vector3f, Vector3f> result = best_rosy_vector_pair(
            this_tangent,
            this_normal,
            other_tangent,
            other_normal);

        float theta = angle_between_vectors(result.first, result.second);
        error += (theta * theta);
    }
    return error;
}

/**
 * @return The current error in a tier of the graph
 */
float
compute_error_for_tier(
    const std::vector<PointNormal::Ptr>& vertices,
    const PointNormalGraphPtr& graph,
    const std::vector<Eigen::Vector3f>& tangents ) {
    using namespace std;

    // E(O, k) :=      (oi, Rso (oji, ni, kij ))
    // For each node
    float error = 0.0f;
    size_t vertex_idx = 0;
    for (auto node : graph->nodes() ) {
        vector<size_t> neighbours = graph->neighbour_indices(node);
        error += compute_error_for_vertex( vertex_idx, neighbours, vertices, tangents );
        vertex_idx++;
    }
    return error;
}

/**
 * Merge PointNormals when simplifying a graph. Each PointNormal has a normal and location.
 * When merging we do the following:
 * Locations are averaged
 * Normals are averaged
 */
PointNormal::Ptr
up_propagate( const PointNormal::Ptr& p1, const PointNormal::Ptr& p2 ) {
    using namespace std;
    using namespace Eigen;

    Vector3f new_point  = (p1->point() + p2->point()) / 2.0;
    Vector3f new_normal = (p1->normal() + p2->normal()).normalized();

    PointNormal::Ptr pn = make_shared<PointNormal>(PointNormal{p1->point(), p1->normal()});
    return pn;
}

/**
 * Do nothing going down as we don't need to propagate any changes
 */
PointNormal::Ptr
down_propagate( const PointNormal::Ptr& parent, const PointNormal::Ptr& child ) {
    return child;
}

/* ******************************************************************************************
 *
 *   Initialisation
 *
 * ******************************************************************************************/

/**
 * Build a graph given a set of PointNormal::Ptrs and adjacency
 */
PointNormalGraphPtr
initialise_tier0_graph(const std::vector<PointNormal::Ptr>& data, const std::multimap<size_t, size_t>& adjacency) {
    using namespace std;

    PointNormalGraphPtr graph = make_shared<PointNormalGraph>();

    for( auto it = adjacency.begin(); it != adjacency.end(); ) {
        size_t vertex_idx = it->first;
        PointNormalGraph::GraphNode * node = graph->add_node( data[vertex_idx] );
        do {
            ++it;
        } while( it != adjacency.end() && it->first == vertex_idx);
    }

    vector<PointNormalGraph::GraphNode *> nodes = graph->nodes( );
    for( auto iter : adjacency ) {
        PointNormalGraph::GraphNode * from = nodes[iter.first];
        PointNormalGraph::GraphNode * to   = nodes[iter.second];
        if( !graph->has_edge( from, to)) {
            graph->add_edge( from, to, 1.0f, nullptr );
        }
    }
    return graph;
}

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
std::pair<std::vector<PointNormalGraphPtr>, std::vector<PointNormalGraphMapping>>
generate_hierarchy(PointNormalGraphPtr graph, int max_tiers, int max_nodes, int max_edges) {
    using namespace std;

    // At least one of max_tiers, max_nodes and max_edges must be >0
    assert (max_edges > 0 || max_nodes > 0 || max_tiers > 0);

    vector<PointNormalGraphPtr> hierarchy;
    vector<PointNormalGraphMapping> mapping_hierarchy;
    hierarchy.push_back( graph );

    bool done = false;
    PointNormalGraphPtr current_tier = graph;
    PointNormalGraphSimplifier * s = new PointNormalGraphSimplifier(up_propagate, down_propagate);

    while (!done) {
        done = done || ((max_nodes > 0) && (current_tier->num_nodes() < max_nodes));
        done = done || ((max_edges > 0) && (current_tier->num_edges() < max_edges));
        done = done || ((max_tiers > 0) && (hierarchy.size() == max_tiers));

        if (!done) {
            pair<PointNormalGraph *, PointNormalGraphMapping> simplify_results = s->simplify(current_tier.get());
            PointNormalGraphPtr next_graph = shared_ptr<PointNormalGraph>(simplify_results.first);
            hierarchy.push_back(next_graph);
            mapping_hierarchy.push_back(simplify_results.second);
            current_tier = next_graph;
        }
    }
    return make_pair( hierarchy, mapping_hierarchy);
}


/**
 * @return the index of a PointNormal::Ptr in a frame/tier
 */
size_t
index_of_point_in( const PointNormal::Ptr& p, const std::vector<PointNormal::Ptr>& v ) {
    using namespace std;

    auto begin = v.begin();
    auto end   = v.end();
    auto it = find(begin, end, p);
    if (it != end) return it - begin;
    throw runtime_error{ "Not found" };
}

/**
 * For a Graph, compute the minimal cycles and then ensure they are oriented
 * clockwise
 */
std::vector<animesh::Path>
compute_cycles(const PointNormalGraphPtr& graph) {
  using namespace std;
  using namespace Eigen;

  vector<animesh::Path> ordered_cycles;
  vector<animesh::Path> cycles = graph->cycles();
  Vector3f z{0, 0, 1};
  for( auto cycle : cycles) {
    // Guaranteed that a cycle has at least 3 nodes
    PointNormal::Ptr p1 = graph->nodes()[cycle[0]]->data();
    PointNormal::Ptr p2 = graph->nodes()[cycle[1]]->data();
    PointNormal::Ptr p3 = graph->nodes()[cycle[2]]->data();

    Vector3f edge1 = p2->point() - p1->point();
    Vector3f edge2 = p3->point() - p2->point();
    if( ( edge1.cross(edge2) ).dot(z) == -1 ) {
      ordered_cycles.push_back( cycle);
    } else {
      ordered_cycles.push_back( cycle.reverse());
    }
  }
  return ordered_cycles;
}

/**
 * For a stack of graphs, compute the minimal cycles and then ensure they are oriented
 * clockwise
 */
std::vector<std::vector<animesh::Path>>
compute_cycles(const std::vector<PointNormalGraphPtr>& graphs) {
  using namespace std;

  vector<vector<animesh::Path>> cycles;
  for( size_t tier_idx = 0; tier_idx < graphs.size(); ++tier_idx) {
    cycles.push_back(compute_cycles(graphs[tier_idx]));
  }
  return cycles;
}

/**
 * Allocate the vectors of vertices for each frame of each tier after tier 0
 */
void
allocate_tiers_and_frames(
          std::vector<std::vector<std::vector<PointNormal::Ptr>>>& tiers,
    const std::vector<PointNormalGraphPtr>&                        graphs) {
    using namespace std;

    size_t num_tiers = graphs.size();
    size_t num_frames = tiers[0].size();

    // Create all the vectors
    for (size_t tier_idx = 1; tier_idx < num_tiers; ++tier_idx) {
        vector<vector<PointNormal::Ptr>> frames_in_this_tier;
        size_t num_vertices = graphs[tier_idx]->num_nodes();
        for( size_t frame_idx = 0; frame_idx < num_frames; ++frame_idx ) {
            frames_in_this_tier.push_back( vector<PointNormal::Ptr>{num_vertices});
        }
        tiers.push_back( frames_in_this_tier);
    }
}

/**
 * On entry:
 *    frames of vertices are allocated for each tier.
 *    graphs is a hierarchy of graphs. The size of which is the number of tiers we need
 * On exit:
 *    We have generated all vertices.
 */
void
populate_tiers_and_frames(
          std::vector<std::vector<std::vector<PointNormal::Ptr>>>& tiers,
    const std::vector<PointNormalGraphPtr>&                        graphs,
    const std::vector<PointNormalGraphMapping>&                    mappings ) {

    using namespace std;

    size_t num_tiers = graphs.size();
    size_t num_frames = tiers[0].size();

    // For each tier (1+) of the graph hierarchy
    for (size_t tier_idx = 1; tier_idx < num_tiers; ++tier_idx) {
        PointNormalGraphPtr graph = graphs[tier_idx];

        // Now add vertex data for all frames
        int parent_idx = 0;
        size_t num_vertices = graph->num_nodes();
        for (size_t vertex_idx = 0; vertex_idx < num_vertices; ++vertex_idx ) {
            auto gn = graph->nodes()[vertex_idx];

            PointNormal::Ptr parent_in_frame_0 = gn->data();
            tiers[tier_idx][0][vertex_idx] = parent_in_frame_0;

            // For each frame after 0
            for (size_t frame_idx = 1; frame_idx < num_frames; ++frame_idx) {
                // Get the children of parent in frame 0 in the previous tier
                vector<PointNormal::Ptr> children_in_frame_0 = mappings[tier_idx - 1].children(gn);
                vector<PointNormal::Ptr> children_in_frame_idx;
                for( auto p : children_in_frame_0 ) {
                    size_t vertex_idx = index_of_point_in(p, tiers[tier_idx-1][0] );
                    children_in_frame_idx.push_back(tiers[tier_idx-1][frame_idx][vertex_idx]);
                }

                // Construct the parent for frame_idx
                PointNormal::Ptr parent_in_frame_idx;
                if( children_in_frame_idx.size() == 1 ) {
                    parent_in_frame_idx = make_shared<PointNormal>( PointNormal{children_in_frame_idx[0]->point(), children_in_frame_idx[0]->normal() } );
                } else if ( children_in_frame_idx.size() == 2 ) {
                    parent_in_frame_idx = up_propagate(children_in_frame_idx[0], children_in_frame_idx[1]);
                } else {
                    throw_runtime_error( "Expected only one or two children" );
                }
                tiers[tier_idx][frame_idx][vertex_idx]= parent_in_frame_idx;
            }
            parent_idx++;
        }
    }
}

/**
 * Given a fully populated set of frames and tiers
 * Compute forward and backwards transforms between frame 0 and frame N in tier N
 */
std::vector<std::vector<std::vector<std::pair<Eigen::Matrix3f, Eigen::Matrix3f>>>>
generate_fwd_and_bkwd_transforms(
    const std::vector<std::vector<std::vector<PointNormal::Ptr>>>& tiers,
    const std::vector<PointNormalGraphPtr>&                        graphs) {

    using namespace std;
    using namespace Eigen;

    size_t num_tiers = tiers.size();
    size_t num_frames = tiers[0].size();
    size_t num_vertices = tiers[0][0].size();

    vector<vector<vector<pair<Matrix3f, Matrix3f>>>> transforms;

    // For each tier
    for (size_t tier_idx = 0; tier_idx < num_tiers; ++tier_idx) {
        vector<vector<pair<Matrix3f, Matrix3f>>> tier_transforms;
        PointNormalGraphPtr graph = graphs[tier_idx];

        // Frame 0 transforms are all identify matrix
        vector<pair<Matrix3f, Matrix3f>> frame_0_transforms;
        for( size_t vertex_idx = 0; vertex_idx < num_vertices; ++vertex_idx) {
            frame_0_transforms.push_back( make_pair(Matrix3f::Identity(), Matrix3f::Identity()));
        }
        tier_transforms.push_back( frame_0_transforms);

        // For remainder of frames we have to compute values
        for( size_t frame_idx = 1; frame_idx < num_frames; ++frame_idx) {
            vector<pair<Matrix3f, Matrix3f>> frame_transforms;

            // For each node in the graph (nodes are in the same order as the )
            size_t vertex_idx = 0;
            for (auto gn : graph->nodes()) {
                // First point and normal
                PointNormal::Ptr pn1 = tiers[tier_idx][0][vertex_idx];
                PointNormal::Ptr pn2 = tiers[tier_idx][frame_idx][vertex_idx];
                Vector3f point1 = pn1->point();
                Vector3f normal1 = pn1->normal();
                Vector3f point2 = pn2->point();
                Vector3f normal2 = pn2->normal();

                vector<size_t> frame_0_neighbour_indices = graph->neighbour_indices(gn);
                vector<Vector3f> neighbour_points_1;
                vector<Vector3f> neighbour_points_2;
                for (size_t vertex_idx : frame_0_neighbour_indices ) {
                    neighbour_points_1.push_back( tiers[tier_idx][0][vertex_idx]->point());
                    neighbour_points_2.push_back( tiers[tier_idx][frame_idx][vertex_idx]->point());
                }
                Matrix3f m = rotation_between(point1, normal1, neighbour_points_1, point2, normal2, neighbour_points_2);
                frame_transforms.push_back(make_pair( m, m.transpose()));
                vertex_idx++;
            }
            tier_transforms.push_back( frame_transforms );
        }
        transforms.push_back( tier_transforms);
    }
    return transforms;
}

/**
 * Generate a set of random but correct tangents for the given tier (and frame-0)
 */
std::vector<Eigen::Vector3f>
generate_random_tangents_for_tier(std::size_t tier_idx, const std::vector<std::vector<std::vector<PointNormal::Ptr>>>& tiers) {
    using namespace std;
    using namespace Eigen;

    assert(tiers.size()>0);
    assert(tiers[tier_idx].size()>0);
    size_t num_vertices = tiers[tier_idx][0].size();
    assert(num_vertices > 0);
    vector<Vector3f> tangents;
    for( size_t vertex_idx = 0; vertex_idx < num_vertices; ++vertex_idx ) {
        Vector3f normal = tiers[tier_idx][0][vertex_idx]->normal();
        Vector3f tangent = (Vector3f::Random().cross(normal)).normalized();
        tangents.push_back(tangent);
    }
    return tangents;
}

/**
 * Build a FieldOptimiser to optimise the given data.
 * Data consists of a number of frames as well as adjacency data.
 * The frame data is assumed to be in correspondence.
 *
 * @param frames The frames of data.
 * @param adjacency A mape describing adjacency. Indices in the map correspond to the point order in frames.
 */
FieldOptimiser::FieldOptimiser(const std::vector<std::vector<PointNormal::Ptr>>& frames, const std::multimap<std::size_t, std::size_t>& adjacency) {
    using namespace std;

    size_t num_frames = frames.size();
    assert( num_frames > 0 );
    size_t num_vertices = frames[0].size();
    assert( num_vertices > 0 );

    // We have the tier 0 frames here. Store them as such
    //vector< /*tiers*/ vector< /*frames*/ vector< /*vertices*/ PointNormal::Ptr> > >
    vector<vector<PointNormal::Ptr>> tier0;
    for( auto frame : frames ) {
        assert( frame.size() == num_vertices);
        tier0.push_back(frame);
    }
    m_tiers.push_back( tier0 );
    m_adjacency = adjacency;

    initialise();
}

/**
 * Return the vertex data for a specific tier and frame
 */
const std::vector<PointNormal::Ptr>&
FieldOptimiser::point_normals_for_tier_and_frame( std::size_t tier_idx, std::size_t frame_idx ) const {
    return m_tiers[tier_idx][frame_idx];
}

/**
 * Return the specific vertex for a tier, frame and vertex index
 */
const PointNormal::Ptr&
FieldOptimiser::point_normal_for_tier_and_frame( std::size_t tier_idx, std::size_t frame_idx, std::size_t vertex_idx ) const {
    return m_tiers[tier_idx][frame_idx][vertex_idx];
}


/**
 * @return a vector of vectors of indices for the neighbours of each point in a tier.
 */
std::vector<std::vector<std::size_t>>
FieldOptimiser::adjacency_for_tier(std::size_t tier_idx) const {
    using namespace std;

    vector<vector<size_t>> adjacency;
    for( auto gn : m_graphs[tier_idx]->nodes()) {
        vector<size_t> neighbours = m_graphs[tier_idx]->neighbour_indices(gn);
        adjacency.push_back(neighbours);
    }

    return adjacency;
}



/**
 * @return the mean edge length in the graph
 */
float
FieldOptimiser::mean_edge_length_for_tier(std::size_t tier_idx ) const {
    float sum = 0.0f;
    for ( auto edge : m_graphs[tier_idx]->edges()) {
        Eigen::Vector3f v1 = edge->from_node( )->data()->point();
        Eigen::Vector3f v2 = edge->to_node( )->data()->point();
        Eigen::Vector3f diff = v2 - v1;
        sum = sum + diff.norm();
    }
    return sum / m_graphs[tier_idx]->num_edges();
}


/**
 * @return The transformation matrix for a specific vertex from frame0 in tier_idx to frame_idx.
 */
const Eigen::Matrix3f&
FieldOptimiser::forward_transform_to( size_t tier_idx, size_t frame_idx, size_t node_idx ) const {
    return m_point_transforms[tier_idx][frame_idx][node_idx].first;
}

/**
 * Initialise the optimiser by:
 * - Generate tier 0 graph (using adjacency)
 * - Generating graph hierarchy (from tier 0)
 * - Generating random tangents (for tier 0)
 * - Computing interframe transforms (for all frames from 0)
 */
void FieldOptimiser::initialise( ) {
    PointNormalGraphPtr graph = initialise_tier0_graph( point_normals_for_tier_and_frame(0,0), m_adjacency);
    auto result = ::generate_hierarchy(graph, SMOOTH_EDGES, SMOOTH_NODES, SMOOTH_TIERS);
    m_graphs = result.first;
    m_mappings = result.second;

    m_cycles = compute_cycles(m_graphs);

    m_is_optimising = false;
    m_optimising_started_new_tier = false;
    m_optimising_last_error = 0.0f;
    m_optimising_iterations_this_tier = 0;
    m_optimising_tier_idx = 0;
    m_optimising_current_tier = nullptr;
    m_tracing_enabled = false;

    allocate_tiers_and_frames(m_tiers, m_graphs);
    populate_tiers_and_frames(m_tiers, m_graphs, m_mappings);
    m_point_transforms = generate_fwd_and_bkwd_transforms(m_tiers, m_graphs );
    // Implicitly tier 0 tangents
    m_tangents = generate_random_tangents_for_tier(0 , m_tiers);
    for( size_t idx = 0; idx < num_frames(); idx++ ) {
        m_include_frames.push_back( true );
    }
}


/**
 * Given a vector of tangents and the current tier index, use the mappings to propagate the tangents upwards through the network
 * by one tier, assuming that we can do so.
 */
std::vector<Eigen::Vector3f>
FieldOptimiser::propagate_tangents_up( const std::vector<Eigen::Vector3f>& tangents, std::size_t tier_idx ) const {
    using namespace std;
    using namespace Eigen;

    assert( tier_idx < m_mappings.size() );

    size_t num_tangents_at_next_tier = m_tiers[tier_idx+1][0].size();
    vector<Vector3f> new_tangents;
    new_tangents.resize( num_tangents_at_next_tier, Vector3f::Zero());

    size_t vertex_idx = 0;
    for( PointNormalGraphNode * gn : m_graphs[tier_idx]->nodes()) {
        const PointNormalGraphNode * pgn = m_mappings[tier_idx].parent(gn);
        size_t pidx = m_graphs[tier_idx+1]->index_of(pgn);
        new_tangents[pidx] = (new_tangents[pidx] + tangents[vertex_idx]);
        vertex_idx++;
    }
    for( size_t tan_idx = 0; tan_idx < num_tangents_at_next_tier; ++tan_idx ) {
        new_tangents[tan_idx] = reproject_to_tangent_space(new_tangents[tan_idx], m_tiers[tier_idx+1][0][tan_idx]->normal());
        new_tangents[tan_idx] = new_tangents[tan_idx].normalized();
    }

    return new_tangents;
}

std::vector<Eigen::Vector3f>
FieldOptimiser::propagate_tangents_down( const std::vector<Eigen::Vector3f>& tangents, std::size_t tier_idx ) const {
    using namespace std;
    using namespace Eigen;

    assert( tier_idx > 0 );

    vector<Vector3f> new_tangents;

    new_tangents.resize( m_tiers[tier_idx-1][0].size(), Vector3f::Zero());
    size_t vertex_idx = 0;
    for( PointNormalGraphNode * gn : m_graphs[tier_idx]->nodes()) {
        vector<PointNormalGraphNode *> children = m_mappings[tier_idx-1].child_nodes(gn);
        for( PointNormalGraphNode * child : children ) {
            size_t pidx = m_graphs[tier_idx-1]->index_of(child);
            new_tangents[pidx] = tangents[vertex_idx];
        }
        vertex_idx++;
    }
    return new_tangents;
}


/**
 * Reproject the tangents from frame 0 tier 0 into an arbitrary frame and tier by using the forward transformations
 * and then reprojecting into tangent plane and normalising.
 * @return The tangents as in tier and frame.
 */
std::vector<Eigen::Vector3f>
FieldOptimiser::compute_tangents_for_tier_and_frame(size_t tier_idx, size_t frame_idx) const {
    using namespace std;
    using namespace Eigen;

    size_t current_tier_idx = m_optimising_tier_idx;
    vector<Vector3f> tier_tangents;
    tier_tangents.insert(end(tier_tangents), begin(m_tangents), end(m_tangents));
    if( current_tier_idx < tier_idx ) {
        while( current_tier_idx < tier_idx ) {
            tier_tangents = propagate_tangents_up( tier_tangents, current_tier_idx );
            current_tier_idx++;
        }
    } else if ( current_tier_idx > tier_idx ) {
        while( current_tier_idx > tier_idx) {
            tier_tangents = propagate_tangents_down( tier_tangents, current_tier_idx );
            current_tier_idx--;
        }
    }

    // FRAME propagation
    vector<Vector3f> frame_tangents;
    if( frame_idx != 0 ) {
        for( size_t vertex_idx = 0; vertex_idx < tier_tangents.size(); ++vertex_idx ) {
            Matrix3f m = forward_transform_to( tier_idx, frame_idx, vertex_idx);
            Vector3f t = tier_tangents[vertex_idx];
            Vector3f new_tan = m * t;
            new_tan = reproject_to_tangent_space( new_tan, m_tiers[tier_idx][frame_idx][vertex_idx]->normal());
            new_tan.normalize();
            frame_tangents.push_back(new_tan);
        }
    }
    else {
        frame_tangents.insert(end(frame_tangents), begin(tier_tangents), end(tier_tangents));
    }
    return frame_tangents;
}


/* ******************************************************************************************
 *
 *   Attributes
 *
 * ******************************************************************************************/
/**
 * @return The number of tiers in the graph hierarchy.
 */
std::size_t
FieldOptimiser::num_tiers() const {
    return m_graphs.size();
}

/**
 * @return The number of frames
 */
std::size_t
FieldOptimiser::num_frames() const {
    return m_tiers[0].size();
}


/**
 * Include or exclude the frame from smoothing.
 */
void
FieldOptimiser::enable_frame(size_t frame_idx, bool enable) {
    m_include_frames[frame_idx] = enable;
}

/**
 * @return true if a frame is included in smoothing, else false.
 */
bool
FieldOptimiser::is_frame_enabled(size_t frame_idx) const {
    return m_include_frames[frame_idx];
}


/**
 * @return the numberof nodes in the given tier.
 */
std::size_t
FieldOptimiser::num_nodes_in_tier( std::size_t tier_idx) const {
    return m_graphs[tier_idx]->num_nodes();
}

/**
 * @return the numberof edges in the given tier.
 */
std::size_t
FieldOptimiser::num_edges_in_tier( std::size_t tier_idx) const {
    return m_graphs[tier_idx]->num_edges();
}

/**
 * @return the optimising index.
 */
size_t
FieldOptimiser::optimising_tier_index( ) const {
    return m_optimising_tier_idx;
}

/**
 * @return true if the optimiser is running.
 */
bool
FieldOptimiser::is_optimising( ) const {
    return m_is_optimising;
}


/* ******************************************************************************************
 *
 *   Smoothing
 *
 * ******************************************************************************************/
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
FieldOptimiser::copy_all_neighbours_for(
    const std::vector<PointNormalGraphPtr>& graphs,
    std::size_t tier_idx,
    std::size_t vertex_idx) const {

    using namespace std;
    using namespace Eigen;

    size_t num_frames = m_tiers[0].size();
    vector<Vector3f>  nbr_normals;
    vector<Vector3f>  nbr_tangents;

    // Get neighbouring vertex indices in this tier at frame 0
    auto node = graphs[tier_idx]->nodes()[vertex_idx];
    vector<size_t> nbr_indices = graphs[tier_idx]->neighbour_indices(node);
    for( size_t nbr_idx : nbr_indices) {
        nbr_normals.push_back( m_tiers[tier_idx][0][nbr_idx]->normal());
        nbr_tangents.push_back(m_tangents[nbr_idx]);
    }

    Vector3f this_vertex_normal = m_tiers[tier_idx][0][vertex_idx]->normal();
    //  Copy temporal neighbours transformed to frame 0
    for (size_t frame_idx = 1; frame_idx < num_frames; ++frame_idx) {
        if(!m_include_frames[frame_idx] )
            continue;

        // Get forward transformed tangents
        vector<Vector3f> tangents_in_frame = compute_tangents_for_tier_and_frame(tier_idx, frame_idx);
        for( size_t nbr_idx : nbr_indices) {
            nbr_normals.push_back( m_tiers[tier_idx][frame_idx][nbr_idx]->normal());

            Matrix3f back_transform = m_point_transforms[tier_idx][frame_idx][vertex_idx].second;
            Vector3f back_transformed_tangent = back_transform * tangents_in_frame[nbr_idx];
            back_transformed_tangent = reproject_to_tangent_space(back_transformed_tangent, this_vertex_normal);
            back_transformed_tangent.normalize();
            nbr_tangents.push_back(back_transformed_tangent);
        }
    }
    return make_pair(nbr_normals, nbr_tangents);
}

/**
 * Update Singularities by:
 * Walk the graph loops for the graph at the current tier and compute 'k' for each edge
 * Sum for each loop and use to identify and locate singularities.
 */
 std::vector<std::tuple<Eigen::Vector3f, Eigen::Vector3f, int>>
 FieldOptimiser::get_singularities_for_tier_and_frame(std::size_t tier_idx, std::size_t frame_idx) const {
   using namespace std;
   using namespace Eigen;

   const vector<PointNormal::Ptr>& pn = point_normals_for_tier_and_frame(tier_idx, frame_idx);
   vector<Vector3f> tangents = compute_tangents_for_tier_and_frame(tier_idx, frame_idx);
   vector<tuple<Vector3f, Vector3f, int>> singularities;

   for( size_t cycle_idx = 0; cycle_idx < m_cycles[tier_idx].size(); ++cycle_idx) {
     const Path& cycle = m_cycles[tier_idx][cycle_idx];
     int total = 0;
     Vector3f singularity_location = Vector3f::Zero();
     Vector3f singularity_normal = Vector3f::Zero();
     
     for( size_t from_idx = 0; from_idx < cycle.length(); ++from_idx) {
       size_t from_node_idx = cycle[from_idx];
       size_t to_node_idx = cycle[ (from_idx + 1 ) % cycle.length()];

       // Compute Rosy counts for from and to
       int to_k = 0, from_k = 0;
       best_rosy_vector_pair( tangents[from_node_idx], pn[from_node_idx]->normal(), from_k,
                              tangents[to_node_idx], pn[to_node_idx]->normal(), to_k);

       // Add to centroid.
       singularity_location = singularity_location + pn[from_node_idx]->point();
       singularity_normal   = singularity_normal   + pn[from_node_idx]->normal();

       total += ((to_k - from_k + 4) % 4);
     }
     if( (total % 4) != 0 ) {
       // Compute singularity centroid
       singularity_location = singularity_location / cycle.length();
       singularity_normal.normalize();

       // Add singularity to list.(centroid and type : 3 or 5)
       singularities.push_back( make_tuple(singularity_location, singularity_normal, total) );
     }
   }
   return singularities;
 }

/**
 * Update the tangents for the given tier of the graph.  The provided vector of tangents is in
 * order specified by indices.
 */
void
FieldOptimiser::update_tangents( const std::vector<Eigen::Vector3f>& new_tangents) {
    using namespace std;

    m_tangents.clear();
    m_tangents.insert( end(m_tangents), begin(new_tangents), end(new_tangents));
}

/**
 * Perform orientation field optimisation.
 * Continuously step until done.
 */
void
FieldOptimiser::optimise() {
    do {
        optimise_do_one_step();
    } while (m_is_optimising);
}

/**
 * Randomise. Can only be performed when the field is not in the process of being optimised.
 */
void
FieldOptimiser::randomise() {


    // TODO: Generate random tangents and propagate across tiers and frames.


}




/**
 * Setup for optimisation. Build the hierarchical graph and
 * construct correspondence maps.
 */
void FieldOptimiser::optimise_begin(){
    using namespace std;
    using namespace Eigen;

    // Assume we're on tier 0; copy tans
    update_tangents( compute_tangents_for_tier_and_frame(m_graphs.size() - 1, 0));

    m_optimising_tier_idx = m_graphs.size() - 1;
    m_optimising_current_tier = m_graphs[m_optimising_tier_idx];
    m_optimising_started_new_tier = true;
    m_is_optimising = true;
    m_optimising_last_error = total_error();
}

/**
 * Start optimising a brand new tier of the hierarchical graph,
 */
void FieldOptimiser::optimise_begin_tier(){
    if (m_tracing_enabled)
        std::cout << "  Level " << m_optimising_tier_idx << std::endl;

    m_optimising_iterations_this_tier = 0;
    m_optimising_started_new_tier = false;
}

bool FieldOptimiser::optimise_end_tier(){
    if (m_optimising_tier_idx != 0) {
        update_tangents(propagate_tangents_down(m_tangents, m_optimising_tier_idx) );
        m_optimising_tier_idx--;
        m_mappings[m_optimising_tier_idx].propagate();
        m_optimising_current_tier = m_graphs[m_optimising_tier_idx];
        m_optimising_started_new_tier = true;
        return false;
    }
    return true;
}

/**
 * Optimisation has concluded.
 */
void FieldOptimiser::optimise_end(){
    m_is_optimising = false;
}

/**
 * Perform a single step of optimisation.
 */
void
FieldOptimiser::optimise_do_one_step() {
    using namespace std;
    using namespace Eigen;

    // If not optimising, perform setup
    if (!m_is_optimising) {
        optimise_begin(); //setup_optimisation();
    }

    // If we're starting a new tier...
    if (m_optimising_started_new_tier) {
        optimise_begin_tier(); //setup_tier_optimisation();
    }

    // Smooth the tier, possible starting a new one
    vector<Vector3f> new_tangents = compute_new_tangents_for_tier(m_graphs, m_optimising_tier_idx);
    update_tangents(new_tangents);
    m_optimising_iterations_this_tier++;
    float new_error = total_error();
    bool is_converged = check_convergence(new_error);
    m_optimising_last_error = new_error;
    if( is_converged ) {
        if( optimise_end_tier() ) {
            optimise_end(); // stop_optimising
        }
    }
}

/**
 * Compute new tangents for all vertices in a tier
 * @param tier_idx The index of the graph tier to be optimised.
 * @return Vector of tangents
 */
std::vector<Eigen::Vector3f>
FieldOptimiser::compute_new_tangents_for_tier(
    const std::vector<PointNormalGraphPtr>& graphs,
    std::size_t tier_idx) const {

    using namespace Eigen;
    using namespace std;

    // Iterate over permute, look up key, lookup fe and smooth
    vector<Eigen::Vector3f> new_tangents;
    for (size_t vertex_idx = 0; vertex_idx < graphs[tier_idx]->num_nodes(); ++vertex_idx) {
        new_tangents.push_back(compute_new_tangent_for_vertex(graphs,tier_idx, vertex_idx));
    }
    return new_tangents;
}


/**
 * Computes the new tangent for a given node by averaging over all neighbours. Final result is
 * projected back into tangent space for the given FE's normal
 * @return The new vector.
 */
Eigen::Vector3f
FieldOptimiser::compute_new_tangent_for_vertex(
    const std::vector<PointNormalGraphPtr>& graphs,
    size_t tier_idx,
    size_t vertex_idx) const {
    using namespace Eigen;
    using namespace std;

    assert( tier_idx == m_optimising_tier_idx);

    PointNormal::Ptr this_vertex = graphs[tier_idx]->nodes()[vertex_idx]->data();

    // Get all neighbours across all frames for this node
    pair<vector<Vector3f>, vector<Vector3f>> neighbour_data = copy_all_neighbours_for( graphs, tier_idx, vertex_idx);
    vector<Vector3f> neighbour_normals = neighbour_data.first;
    vector<Vector3f> neighbour_tangents = neighbour_data.second;


    // Merge all neighbours; implicitly using optiminsing tier tangents
    Vector3f new_tangent = m_tangents[vertex_idx];
    float weight = 0;
    for ( size_t neighbour_idx = 0; neighbour_idx < neighbour_normals.size(); ++neighbour_idx) {
        // TODO: Extract the edge weight from the graph node
        float edge_weight = 1.0f;
        new_tangent = average_rosy_vectors(new_tangent, this_vertex->normal(), weight,
                                           neighbour_tangents[neighbour_idx], neighbour_normals[neighbour_idx],
                                           edge_weight);
        weight += edge_weight;
    }
    return new_tangent;
}

/* ******************************************************************************************
 *
 *   Error calculations
 *
 * ******************************************************************************************/

/**
 * @return The current error. This is calculated on the current tier when smoothing and tier0 when not.
 */
float
FieldOptimiser::total_error() const {
    using namespace std;
    using namespace Eigen;

    size_t tier_idx = 0;
    if(m_is_optimising) {
        tier_idx = m_optimising_tier_idx;
    }
    PointNormalGraphPtr graph = m_graphs[tier_idx];
    vector<Vector3f> tangents = compute_tangents_for_tier_and_frame(tier_idx,0);

    float error = compute_error_for_tier( m_tiers[tier_idx][0], graph, tangents );
    return error;
}

/**
 * @return true if the optimisation operation has converged
 * (or has iterated enough times)
 * otherwise return false
 */
bool
FieldOptimiser::check_convergence(float new_error) const {
    float delta = m_optimising_last_error - new_error;
    float pct = delta / m_optimising_last_error;
    float display_pct = std::floor(pct * 1000.0f) / 10.0f;

    bool converged = (display_pct >= 0.0f && display_pct < CONVERGENCE_THRESHOLD);
    if (m_tracing_enabled) {
        std::cout << "      New Error : " << new_error << " (" << delta << ") : " << display_pct << "%%"
                  << std::endl;
    }

    if (converged) {
        if (m_tracing_enabled) {
            std::cout << "      Converged" << std::endl;
        }
    } else {
        if (m_optimising_iterations_this_tier == MAX_ITERS_PER_TIER) {
            converged = true;
            if (m_tracing_enabled) {
                std::cout << "      Not converging. skip to next tier" << std::endl;
            }
        }
    }
    return converged;
}
