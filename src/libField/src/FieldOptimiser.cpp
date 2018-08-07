#include <Field/FieldOptimiser.h>
#include <Field/Field.h>
#include <RoSy/RoSy.h>

const int MAX_ITERS_PER_TIER = 20;
const float CONVERGENCE_THRESHOLD = 0.5f;
const int SMOOTH_EDGES = 10;
const int SMOOTH_NODES = 20;
const int SMOOTH_TIERS = 10;

using animesh::Field;
using animesh::FieldGraph;
using animesh::FieldElement;
using animesh::FieldOptimiser;
using animesh::FieldGraphSimplifier;
using animesh::CorrespondenceMapping;

/**
 * Construct with a Field to be optimised
 */
FieldOptimiser::FieldOptimiser(Field *field) {
    if (field == nullptr) throw std::invalid_argument("Field cannot be null");
    m_field = field;
    m_graph_hierarchy.push_back(m_field->m_graph);
    m_tracing_enabled = true;
}

size_t FieldOptimiser::index(size_t frame_idx, size_t tier_idx) const {
    if (frame_idx > m_field->get_num_frames())
        throw std::runtime_error("frame_idx is invalid");
    if (tier_idx > m_graph_hierarchy.size()) throw std::runtime_error("tier_idx is invalid");

    size_t idx = tier_idx * m_field->get_num_frames() + frame_idx;
    return idx;
}


std::vector<FieldElement*>
FieldOptimiser::get_corresponding_fes_in_frame(size_t frame_idx, size_t  tier_idx, std::vector<FieldElement*> fes) const {
    std::vector<FieldElement*> vec;
    for( FieldElement * fe : fes ) {
        vec.push_back(const_cast<FieldElement*>(get_corresponding_fe_in_frame( frame_idx, tier_idx, fe)));
    }
    return vec;
}

/**
 * @return the FE corresponding to the given one in a given tier and frame
 */
const FieldElement*
FieldOptimiser::get_corresponding_fe_in_frame( size_t frame_idx, size_t tier_idx, const FieldElement* src_fe  ) const {
    using namespace std;

    size_t idx = index(frame_idx, tier_idx);

    vector<FieldElement*> source = m_field_element_mappings[index(0, tier_idx)];
    vector<FieldElement*> dest   = m_field_element_mappings[index(frame_idx, tier_idx)];

    auto it = find(source.begin(), source.end(), src_fe);
    assert( it != source.end());
    return dest[it - source.begin()];
}


void 
FieldOptimiser::build_equivalent_fes( ) {
    using namespace std;

    size_t num_frames = m_field->get_num_frames();
    size_t num_tiers = m_graph_hierarchy.size();

    // Allocate a bunch of storage
    m_field_element_mappings = new vector<FieldElement*>[num_frames * num_tiers];

    // Copy the tier zero stuff directly from field
    for( size_t frame_idx = 0; frame_idx < num_frames; ++frame_idx ) {
        size_t idx = index(frame_idx,0);
        m_field_element_mappings[idx].insert(m_field_element_mappings[idx].end(),m_field->elements(idx).begin(), m_field->elements(idx).end() );
    }

    // For each other tier of the graph hierarchy
    for (size_t tier_idx = 1; tier_idx < m_graph_hierarchy.size(); ++tier_idx) {
        FieldGraph *fg = m_graph_hierarchy[tier_idx];

        // For each node; sort frame 0
        for (auto gn : fg->nodes()) {
            FieldElement *frame_0_parent_fe = gn->data();

            // Stash the base node in frame 0
            m_field_element_mappings[index(0, tier_idx)].push_back(frame_0_parent_fe);

            for (size_t frame_idx = 1; frame_idx < num_frames; ++frame_idx) {
        
                // Get the children (at time t0) in the previous tier
                vector<FieldElement *> frame_0_children = m_mapping_hierarchy[tier_idx - 1].children(gn);

                // Get corresponding elements in specified frame
                vector<FieldElement *> frame_idx_children = get_corresponding_fes_in_frame(frame_idx, tier_idx-1, frame_0_children);

                FieldElement * frame_idx_parent;
                if( frame_idx_children.size() == 1 ) {
                    frame_idx_parent = new FieldElement(frame_idx_children[0]->location(), frame_idx_children[0]->normal() );
                } else if ( frame_idx_children.size() == 2 ) {
                    frame_idx_parent = FieldElement::mergeFieldElements(frame_idx_children[0], frame_idx_children[1]);
                } else {
                    string err = "Expected one or two children but found " + std::to_string(frame_0_children.size());
                    throw runtime_error(err);
                }
                m_field_element_mappings[ index(frame_idx, tier_idx)].push_back( frame_idx_parent );
            }
        }
    }
}

void 
FieldOptimiser::build_transforms( ) {
    using namespace std;
    using namespace Eigen;

    size_t num_frames = m_field->get_num_frames();
    size_t num_tiers = m_graph_hierarchy.size();

    m_transforms = new vector<Eigen::Matrix3f>[num_frames * num_tiers];

    // For each tier
    for (size_t tier_idx = 0; tier_idx < num_tiers; ++tier_idx) {
        FieldGraph *fg = m_graph_hierarchy[tier_idx];

        // Frame 0 transforms are all identify matrix
        for( size_t i = 0; i < m_graph_hierarchy[0]->num_nodes(); ++i) {
            m_transforms[index(0,tier_idx)].push_back( Matrix3f::Identity());
        }

        // For remainder of frames we have to compute values
        for( size_t frame_idx = 1; frame_idx < num_frames; ++frame_idx) {
            size_t node_idx = 0;
            for (auto gn : fg->nodes()) {
                // 1. Get the neighbours of this FE according to graph
                FieldElement *fe = gn->data();
                vector<FieldElement *> fe_neighbours = fg->neighbours_data(gn);
                const FieldElement * other_fe = get_corresponding_fe_in_frame( frame_idx, tier_idx, fe );
                vector<FieldElement *> other_fe_neighbours = get_corresponding_fes_in_frame( frame_idx, tier_idx, fe_neighbours );

                // First point and normal
                Vector3f point1 = fe->location();
                Vector3f normal1 = fe->normal();

                // Second point and normal
                Vector3f point2 = other_fe->location();
                Vector3f normal2 = other_fe->normal();

                // Find points for neighbours at both time intervals
                vector<Vector3f> neighbour_points_1;
                vector<Vector3f> neighbour_points_2;
                for (size_t i=0; i< fe_neighbours.size(); ++i ) {
                    neighbour_points_1.push_back(fe_neighbours[i]->location());
                    neighbour_points_2.push_back(other_fe_neighbours[i]->location());
                }
                Matrix3f m = rotation_between(point1, normal1, neighbour_points_1, point2, normal2, neighbour_points_2);
                cout << "M (" << frame_idx << ", " << tier_idx  << ", " << node_idx << ")"<< endl << m << endl;
                node_idx++;
                m_transforms[index(frame_idx, tier_idx)].push_back( m );
            }
        }
    }
}

/**
 * Expectations:
 * m_graph_hierarchy is setup and has multiple tiers
 * For each tier of the graph hierarchy, build an equivalent set of correspondences
 * to the newly generated FEs
 */
void
FieldOptimiser::build_correspondences() {
    using namespace std;

    if(m_tracing_enabled) cout << "build_correspondences" << endl;

    if (m_graph_hierarchy.size() == 0)
        throw runtime_error("build_correspondences() called with no graph hierarchy");

    build_equivalent_fes( );

    build_transforms( );
}

/**
 * Setup for optimisation. Build the hierarchical graph and
 * construct correspondence maps.
 */
void
FieldOptimiser::setup_optimisation() {
    generate_hierarchy(SMOOTH_EDGES, SMOOTH_NODES, SMOOTH_TIERS);
    build_correspondences();

    m_optimising_tier_index = m_graph_hierarchy.size() - 1;;
    m_optimising_current_tier = m_graph_hierarchy[m_optimising_tier_index];
    m_optimising_started_new_tier = true;
    m_is_optimising = true;

    m_optimising_last_error = calculate_error(m_optimising_current_tier);
    if (m_tracing_enabled)
        std::cout << "Starting smooth. Error : " << m_optimising_last_error << std::endl;
}

/**
 * Mark optimisation as done.
 */
void
FieldOptimiser::stop_optimising() {
    m_is_optimising = false;
    size_t tier = m_graph_hierarchy.size() - 1;
    while (tier > 0) {
        delete m_graph_hierarchy[tier];
        tier--;
    }

    delete [] m_transforms;
    delete [] m_field_element_mappings;
    m_graph_hierarchy.clear();
    m_mapping_hierarchy.clear();
    m_graph_hierarchy.push_back(m_field->m_graph);
}

/**
 * Start optimising a brand new tier of the hierarchical graph,
 */
void
FieldOptimiser::setup_tier_optimisation() {
    if (m_tracing_enabled)
        std::cout << "  Level " << m_optimising_tier_index << std::endl;

    m_optimising_iterations_this_tier = 0;
    m_optimising_started_new_tier = false;
}


/**
 */
void FieldOptimiser::update_tangents( size_t tier_idx, const std::vector<Eigen::Vector3f> new_tangents ) {
    using namespace Eigen;

    assert( tier_idx < m_graph_hierarchy.size() );
    FieldGraph * fg = m_graph_hierarchy[tier_idx];

    size_t num_frames = m_field->get_num_frames();

    for( size_t node_idx = 0; node_idx < fg->num_nodes(); ++ node_idx ) {
        Vector3f new_tan = new_tangents[node_idx];

        // Set the frames
        for( size_t frame_idx = 0; frame_idx < num_frames; ++frame_idx ) {
            size_t idx = index(frame_idx, tier_idx);
            if( frame_idx == 0 ) {
                m_field_element_mappings[idx][node_idx]->set_tangent( new_tan);
            } else {
                Matrix3f m = m_transforms[idx][node_idx];
                m_field_element_mappings[idx][node_idx]->set_tangent( m * new_tan);
            }
        }
    }
}
/**
 * Smooth the current tier of the hierarchy once and return true if it converged
 * @param tier The Graph (tier) to be optimised
 */
bool
FieldOptimiser::optimise_tier_once(FieldGraph *tier) {
    using namespace Eigen;
    using namespace std;

    if (m_tracing_enabled) cout << "    smooth_once" << endl;

    // Extract map keys into vector and shuffle
    vector<FieldGraphNode *> nodes = tier->nodes();
    random_shuffle(nodes.begin(), nodes.end());

    // Iterate over permute, look up key, lookup fe and smooth
    vector<Vector3f> new_tangents;
    for (auto node : nodes) {
        new_tangents.push_back(calculate_smoothed_node(tier, node));
    }

    // Now update all of the nodes
    update_tangents( m_optimising_tier_index, new_tangents );

    // Get the new error
    m_optimising_iterations_this_tier++;
    float new_error = calculate_error(tier);
    return check_convergence(new_error);
}


/**
 * Perform a single step of optimisation.
 */
void
FieldOptimiser::optimise_one_step() {
    // If not optimising, perform setup
    if (!m_is_optimising) {
        setup_optimisation();
    }

    // If we're starting a new tier...
    if (m_optimising_started_new_tier) {
        setup_tier_optimisation();
    }

    // Smooth the tier, possible starting a new one
    if (optimise_tier_once(m_optimising_current_tier)) {
        // Converged. If there's another tier, do it
        if (m_optimising_tier_index != 0) {

            m_optimising_tier_index--;
            m_mapping_hierarchy[m_optimising_tier_index].propagate();
            m_optimising_current_tier = m_graph_hierarchy[m_optimising_tier_index];
            m_optimising_started_new_tier = true;
        }

        // Otherwise, done
        else {
            stop_optimising();
        }
    }
}

/**
 * Perform orientation field optimisation. Continuously step until done.
 */
void
FieldOptimiser::optimise() {
    do {
        optimise_one_step();
    } while (m_is_optimising);
}

/**
 * @return true if the optimisation operation has converged
 * (or has iterated enough times)
 * otherwise return false
 */
bool
FieldOptimiser::check_convergence(float new_error) {
    float delta = m_optimising_last_error - new_error;
    float pct = delta / m_optimising_last_error;
    float display_pct = std::floor(pct * 1000.0f) / 10.0f;
    m_optimising_last_error = new_error;

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

/**
 * Smooth the specified node
 * @return The new vector.
 */
Eigen::Vector3f 
FieldOptimiser::calculate_smoothed_node(FieldGraph *tier, FieldGraphNode *gn) const {
    using namespace Eigen;
    using namespace std;

    FieldElement *this_fe = (FieldElement *) gn->data();
    if (m_tracing_enabled) std::cout << "smooth_node" << this_fe << std::endl;

    // TODO: Replace this by passing in node index in future
    vector<FieldElement*> fem = m_field_element_mappings[index(0, m_optimising_tier_index)];
    auto it = find(fem.begin(), fem.end(), this_fe);
    assert( it != fem.end());
    size_t node_idx = it - fem.begin();

    vector<FieldElement *> spatial_neighbours = tier->neighbours_data(gn);

    Vector3f new_tangent = this_fe->tangent();
    float weight = 0;

    // Merge spatial neighbours
    for (auto neighbour_fe : spatial_neighbours) {
        if (m_tracing_enabled) std::cout << "    consider neighbour" << neighbour_fe << std::endl;

        // Find best matching rotation
        // TODO: Extract the edge weight from the graph node
        float edge_weight = 1.0f;
        new_tangent = average_rosy_vectors(new_tangent, this_fe->normal(), weight,
                                           neighbour_fe->tangent(), neighbour_fe->normal(), edge_weight);
        weight += edge_weight;
    }

    // Merge temporal neighbours
    for (size_t frame_idx = 1; frame_idx < m_field->get_num_frames(); ++frame_idx) {

        // Get my own transformation matrix at this time point
        Matrix3f m = m_transforms[index(frame_idx, m_optimising_tier_index)][node_idx];
        Matrix3f minv = m.inverse();

        // Now for each spatial neighbour
        vector<FieldElement*> temporal_neighbours = get_corresponding_fes_in_frame(frame_idx, m_optimising_tier_index, spatial_neighbours);
        for (auto future_neighbour : temporal_neighbours) {

            // back project the future coord to the now using m inverse
            Vector3f future_tangent = minv * future_neighbour->tangent();
            Vector3f future_normal = minv * future_neighbour->normal();

            new_tangent = average_rosy_vectors(new_tangent, this_fe->normal(), 0.5f,
                                               future_tangent, future_normal, 0.5f);
        }
    }


    return new_tangent;
}

std::vector<FieldElement*> 
FieldOptimiser::get_elements_at( size_t frame_idx, size_t tier_idx ) const {
    assert( m_field != nullptr);
    std::cout << "get_elements_at( " << frame_idx << ", " << tier_idx << ")" << std::endl;

    // If the graph has expanded, use local vars, otherwise use field vars
    if(m_graph_hierarchy.size() == 1 ) {
        assert( tier_idx == 0 );
        return m_field->elements(frame_idx );
    } else {
        return m_field_element_mappings[ index( frame_idx, tier_idx)];    
    }
}

/**
 * Current error in field
 */
float FieldOptimiser::current_error(int tier) const {
    return calculate_error(graph_at_tier(tier));
}


/**
 * @return the smoothness of the entire Field
 */
float FieldOptimiser::calculate_error(FieldGraph *tier) const {
    // E(O, k) :=      (oi, Rso (oji, ni, kij ))
    // For each node
    float error = 0.0f;
    for (auto node : tier->nodes()) {
        error += calculate_error_for_node(tier, node);
    }
    return error;
}

/**
 * @return the smoothness of one node
 */
float FieldOptimiser::calculate_error_for_node(FieldGraph *tier, FieldGraphNode *gn) const {
    float error = 0.0f;

    FieldElement *this_fe = (FieldElement *) gn->data();

    std::vector<FieldGraphNode *> neighbours = tier->neighbours(gn);

    for (auto n : neighbours) {

        FieldElement *neighbour_fe = n->data();

        std::pair<Eigen::Vector3f, Eigen::Vector3f> result = best_rosy_vector_pair(
                    this_fe->tangent(),
                    this_fe->normal(),
                    neighbour_fe->tangent(),
                    neighbour_fe->normal());

        float theta = angle_between_vectors(result.first, result.second);
        error += (theta * theta);
    }
    return error;
}


/**
 * @Return the nth graph in the hierarchy where 0 is base.
 */
FieldGraph *FieldOptimiser::graph_at_tier(std::size_t tier) const {
    if (tier >= m_graph_hierarchy.size()) throw std::invalid_argument("Tier out of range");
    return m_graph_hierarchy[tier];
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
void FieldOptimiser::generate_hierarchy(int max_tiers, int max_nodes, int max_edges) {
    if (m_graph_hierarchy.size() > 1)
        throw std::runtime_error("Hierarchy already generated");

    // At least one of max_tiers, max_nodes and max_edges must be >0
    if (max_edges <= 0 && max_nodes <= 0 && max_tiers <= 0) {
        throw std::invalid_argument("Must specify terminating criteria for hierarchy generation");
    }

    if (m_tracing_enabled) {
        std::cout << "Generating graph hierarchy. Teminating when one of ";
        if (max_edges > 0) {
            std::cout << " edges <= " << max_edges;
        }
        if (max_nodes > 0) {
            std::cout << " nodes <= " << max_nodes;
        }
        if (max_tiers > 0) {
            std::cout << " tiers <= " << max_tiers;
        }
        std::cout << " is true" << std::endl;
        std::cout << "  Start :" << m_graph_hierarchy[0]->num_nodes() << " nodes, "
                  << m_graph_hierarchy[0]->num_edges() << " edges" << std::endl;
    }


    bool done = false;
    FieldGraph *current_tier = m_graph_hierarchy[0];
    FieldGraphSimplifier *s = new FieldGraphSimplifier(FieldElement::mergeFieldElements,
            FieldElement::propagateFieldElements);

    while (!done) {
        done = done || ((max_nodes > 0) && (current_tier->num_nodes() < max_nodes));
        done = done || ((max_edges > 0) && (current_tier->num_edges() < max_edges));
        done = done || ((max_tiers > 0) && (m_graph_hierarchy.size() == max_tiers));

        if (!done) {
            std::pair<FieldGraph *, FieldGraphMapping> simplify_results = s->simplify(current_tier);
            m_graph_hierarchy.push_back(simplify_results.first);
            m_mapping_hierarchy.push_back(simplify_results.second);
            current_tier = simplify_results.first;
        }

        if (m_tracing_enabled)
            std::cout << "  Tier : " << m_graph_hierarchy.size() << "Nodes :" << current_tier->num_nodes()
                      << ", Edges :" << current_tier->num_edges() << std::endl;
    }
}

/**
 * @Return the current tier being optimised or 0 if none
 */
size_t
FieldOptimiser::optimising_tier_index() const {
    return m_is_optimising ? m_optimising_tier_index : 0;
}