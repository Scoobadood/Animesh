#include <Field/FieldOptimiser.h>
#include <Field/Field.h>
#include <RoSy/RoSy.h>

const int MAX_ITERS_PER_TIER = 20;
const float CONVERGENCE_THRESHOLD = 0.5f;
const int SMOOTH_EDGES = 10;
const int SMOOTH_NODES = 20;
const int SMOOTH_TIERS = 10;


namespace animesh {

/**
 * Construct with a Field to be optimised
 */
    FieldOptimiser::FieldOptimiser(Field *field) {
        if (field == nullptr) throw std::invalid_argument("Field cannot be null");
        m_field = field;
        m_graph_hierarchy.push_back(m_field->m_graph);
    }

/**
 * Start optimising.
 */
    void FieldOptimiser::start_optimising() {
        generate_hierarchy(SMOOTH_EDGES, SMOOTH_NODES, SMOOTH_TIERS);

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
    void FieldOptimiser::stop_optimising() {
        m_is_optimising = false;
        size_t tier = m_graph_hierarchy.size() - 1;
        while (tier > 0) {
            delete m_graph_hierarchy[tier];
            tier--;
        }
        m_graph_hierarchy.clear();
        m_mapping_hierarchy.clear();
        m_graph_hierarchy.push_back(m_field->m_graph);
    }

/**
 * Start a brand ew optimisation session
 */
    void FieldOptimiser::start_optimising_tier() {
        if (m_tracing_enabled)
            std::cout << "  Level " << m_optimising_tier_index << std::endl;

        m_optimising_iterations_this_tier = 0;
        m_optimising_started_new_tier = false;
    }

/**
 * Smooth the current tier of the hierarchy once and return true if it converged
 * @param tier The Graph (tier) to be optimised
 */
    bool FieldOptimiser::optimise_tier_once(FieldGraph *tier) {
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
 * Optimize the field
 */
    void FieldOptimiser::optimise_once() {
        // If not optimising, start
        if (!m_is_optimising) {
            start_optimising();
        }

        // If we're starting a new tier...
        if (m_optimising_started_new_tier) {
            start_optimising_tier();
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
 * Smooth the field
 */
    void FieldOptimiser::optimise() {
        // Debounce
        if (m_is_optimising)
            return;

        do {
            optimise_once();
        } while (m_is_optimising);
    }

/**
 * @return true if the optimisation operation has converged
 * (or has iterated enough times)
 * otherwise return false
 */
    bool FieldOptimiser::check_convergence(float new_error) {
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
        for (size_t tp_idx = 0; tp_idx < m_field->get_num_timepoints(); ++tp_idx) {
            // Get my own transformation matrix at this time point
            Matrix3f m = m_field->get_fwd_xform_for(this_fe, tp_idx);
            Matrix3f minv = m.inverse();

            vector<FieldElement *> temporal_neighbours = m_field->get_neighbours_of(this_fe, tp_idx);

            // Now for each spatial neighbour
            for (auto neighbour_fe : temporal_neighbours) {
                // back project the future coord to the now using m inverse
                Vector3f future_tangent = minv * neighbour_fe->tangent();
                Vector3f future_normal = minv * neighbour_fe->normal();

                new_tangent = average_rosy_vectors(new_tangent, this_fe->normal(), 0.5f,
                                                   future_tangent, future_normal, 0.5f);
            }
        }


        return new_tangent;
    }

/* ********** 
 * * Error Computations
 * *****/
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
    size_t FieldOptimiser::optimising_tier_index() const {
        return m_is_optimising ? m_optimising_tier_index : 0;
    }


}