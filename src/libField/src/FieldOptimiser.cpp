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

}

size_t FieldOptimiser::index(size_t frame_idx, size_t tier_idx) const {
    if (frame_idx > m_field->get_num_frames())
        throw std::runtime_error("frame_idx is invalid");
    if (tier_idx > m_graph_hierarchy.size()) throw std::runtime_error("tier_idx is invalid");
    if (m_correspondences == nullptr) throw std::runtime_error("correspondences not allocated");

    size_t idx = tier_idx * m_field->get_num_frames() + frame_idx;
    return idx;
}

/**
 * Return a reference to the correspondence mapping for the given tier and frame
 * @param frame_idx
 * @param tier_idx
 * @return
 */
CorrespondenceMapping*
FieldOptimiser::get_correspondence_mapping_at(size_t frame_idx, size_t tier_idx) const {
    return m_correspondences[index(frame_idx, tier_idx)];
}

/**
* Set the correspondence mapping for the given tier and frame
* @param frame_idx
* @param tier_idx
* @return
*/
void
FieldOptimiser::set_correspondence_mapping(size_t frame_idx, size_t tier_idx, CorrespondenceMapping * mapping) {
    if (frame_idx > m_field->get_num_frames())
        throw std::runtime_error("frame_idx is invalid");
    if (tier_idx > m_graph_hierarchy.size()) throw std::runtime_error("tier_idx is invalid");
    if (m_correspondences == nullptr) throw std::runtime_error("correspondences not allocated");

    size_t idx = tier_idx * m_field->get_num_frames() + frame_idx;
    m_correspondences[idx] = mapping;
}

/**
 * @return the FE corresponding to the given one in a given tier and frame
 */
const FieldElement*
FieldOptimiser::get_corresponding_fe_in_frame( size_t frame_idx, size_t tier_idx, const FieldElement* src_fe  ) const {
    if (frame_idx > m_field->get_num_frames())
        throw std::runtime_error("frame_idx is invalid");
    if (tier_idx > m_graph_hierarchy.size()) throw std::runtime_error("tier_idx is invalid");
    
    if( tier_idx == 0 ) {
        if( frame_idx == 0 ) return src_fe;

        auto it = std::find(m_field->m_frame_data[0].begin(), m_field->m_frame_data[0].end(), src_fe);
        assert( it != m_field->m_frame_data[0].end());
        return m_field->m_frame_data[frame_idx][it - m_field->m_frame_data[0].begin()];
    } 
    if (m_correspondences == nullptr) throw std::runtime_error("correspondences not allocated");
    size_t idx = tier_idx * m_field->get_num_frames() + frame_idx;
    CorrespondenceMapping * mapping = get_correspondence_mapping_at(frame_idx, tier_idx);
    return mapping->get_corresponding_fe(const_cast<FieldElement*>(src_fe));
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

    size_t num_frames = m_field->get_num_frames();
    size_t num_tiers = m_graph_hierarchy.size();


    // Allocate a bunch of storage
    m_correspondences = new CorrespondenceMapping*[num_frames * num_tiers];

    for( size_t frame_idx = 0; frame_idx < num_frames; ++frame_idx ) {
        m_correspondences[frame_idx] = new CorrespondenceMapping(
            m_field->elements(0), 
            m_field->elements(0), 
            m_graph_hierarchy[0]);
    }

    // For each tier of the graph hierarchy
    for (size_t tier_idx = 1; tier_idx < m_graph_hierarchy.size(); ++tier_idx) {
        FieldGraph *fg = m_graph_hierarchy[tier_idx];
        vector<FieldElement*> firstFrameElements;

        // Get first frame elemenets
        for (auto gn : fg->nodes()) {
            firstFrameElements.push_back(gn->data());
        }

        // for each subsequent frame
        for (size_t frame_idx = 0; frame_idx < num_frames; ++frame_idx) {
            vector<FieldElement*> currentFrameElements;
            CorrespondenceMapping * mapping = get_correspondence_mapping_at(frame_idx, tier_idx-1);

            // For each node
            for (auto gn : fg->nodes()) {
                FieldElement *parent_fe = gn->data();
                // Get the children (at time t0) in the previous tier
                vector<FieldElement *> child_fes = m_mapping_hierarchy[tier_idx - 1].children(gn);

                // Find the childrens' positions at that frame
                vector<FieldElement *> future_child_fes;
                for (FieldElement *fe : child_fes) {
                    future_child_fes.push_back( const_cast<FieldElement*>(mapping->get_corresponding_fe(fe)));
                }
                // Compute mean of children
                FieldElement *future_parent_fe;
                if (future_child_fes.size() == 1) {
                    future_parent_fe = new FieldElement(future_child_fes[0]->location(),
                                                        future_child_fes[0]->normal());
                } else if (child_fes.size() == 2) {
                    future_parent_fe = FieldElement::mergeFieldElements(future_child_fes[0], future_child_fes[1]);
                } else {
                    string err = "Expected one or two children but found " + std::to_string(child_fes.size());
                    throw runtime_error(err);
                }

                currentFrameElements.push_back(future_parent_fe);
            }
            m_correspondences[(tier_idx * num_frames + frame_idx)] = new CorrespondenceMapping(
                firstFrameElements, currentFrameElements, fg);
        }
    }
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
    m_graph_hierarchy.clear();
    m_mapping_hierarchy.clear();
    delete[] m_correspondences;
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
 * Smooth the current tier of the hierarchy once and return true if it converged
 * @param tier The Graph (tier) to be optimised
 */
bool
FieldOptimiser::optimise_tier_once(FieldGraph *tier) {
    using namespace Eigen;
    using namespace std;

    if (m_tracing_enabled) cout << "    smooth_once" << endl;

    // Extract map keys into vector and shuffle
    std::vector<FieldGraphNode *> nodes = tier->nodes();
    random_shuffle(nodes.begin(), nodes.end());

    // Iterate over permute, look up key, lookup fe and smooth
    vector<const Vector3f> new_tangents;
    for (auto node : nodes) {
        new_tangents.push_back(calculate_smoothed_node(tier, node));
    }

    // Now update all of the nodes
    auto tan_iter = new_tangents.begin();
    for (auto node : nodes) {
        node->data()->set_tangent(*tan_iter);
        ++tan_iter;
    }

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
        std::cout << "      New Error : " << new_error << " (" << delta << ") : " << display_pct << "%"
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
Eigen::Vector3f FieldOptimiser::calculate_smoothed_node(FieldGraph *tier, FieldGraphNode *gn) const {
    using namespace Eigen;
    using namespace std;

    FieldElement *this_fe = (FieldElement *) gn->data();
    if (m_tracing_enabled) std::cout << "smooth_node" << this_fe << std::endl;

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
    for (size_t frame_idx = 0; frame_idx < m_field->get_num_frames(); ++frame_idx) {
        CorrespondenceMapping * cm = get_correspondence_mapping_at(frame_idx, m_optimising_tier_index);
        // Get my own transformation matrix at this time point
        Matrix3f m = cm->get_transformation_for(this_fe);
        Matrix3f minv = m.inverse();

        // Now for each spatial neighbour
        for (auto neighbour_fe : spatial_neighbours) {
            // Find the corresponding point at the given frame
            const FieldElement *future_neighbour_fe = cm->get_corresponding_fe(neighbour_fe);

            // back project the future coord to the now using m inverse
            Vector3f future_tangent = minv * future_neighbour_fe->tangent();
            Vector3f future_normal = minv * future_neighbour_fe->normal();

            new_tangent = average_rosy_vectors(new_tangent, this_fe->normal(), 0.5f,
                                               future_tangent, future_normal, 0.5f);
        }
    }


    return new_tangent;
}



std::vector<FieldElement*> 
FieldOptimiser::get_elements_at( size_t frame_idx, size_t tier_idx ) const {
    using namespace std;
    vector<FieldElement*> elements;

    if( tier_idx == 0 ) {
        elements.insert( elements.end(), m_field->m_frame_data[frame_idx].begin(), m_field->m_frame_data[frame_idx].end());
    } else {
        FieldGraph *fg = graph_at_tier(tier_idx);
        animesh::CorrespondenceMapping* cm = get_correspondence_mapping_at(frame_idx, tier_idx);
        for (auto gn : fg->nodes()) {
            FieldElement *fe = gn->data();
            FieldElement *frame_fe = const_cast<FieldElement*>(get_corresponding_fe_in_frame(frame_idx, tier_idx, fe));
            frame_fe->set_tangent( cm->get_transformation_for(fe) * fe->tangent() );
            elements.push_back( frame_fe);
        }
    }

    return elements;

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