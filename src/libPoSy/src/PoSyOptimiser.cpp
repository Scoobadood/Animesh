//
// Created by Dave Durbin on 18/5/20.
//

#include "PoSyOptimiser.h"

#include <utility>
#include <sys/stat.h>
#include <algorithm>
#include <random>
#include <PoSy.h>
#include <Optimise/SurfelSelectionAlgorithm.h>
#include <Eigen/Geometry>

PoSyOptimiser::PoSyOptimiser(Properties properties)
        :
          m_termination_criteria{0} //
        , m_term_crit_absolute_smoothness{0.0f} //
        , m_term_crit_relative_smoothness{0.0f} //
        , m_term_crit_max_iterations{0} //
        , m_result{NOT_COMPLETE} //
        , m_properties(std::move(properties)) //
        , m_convergence_threshold{1.0} //
        , m_num_frames{0} //
        , m_last_smoothness{0.0f} //
        , m_state{UNINITIALISED} //
        , m_num_iterations{0} //
{
    m_rho = m_properties.getFloatProperty("rho");
    m_convergence_threshold = m_properties.getFloatProperty("convergence-threshold");
    m_node_selection_function = extractSsa(m_properties);

    setup_termination_criteria();
}

PoSyOptimiser::~PoSyOptimiser() = default;

/**
 * Start global smoothing.
 */
void
PoSyOptimiser::optimise_begin() {
    assert(m_state == INITIALISED);

    using namespace spdlog;
    info("Initialising error value");
    m_last_smoothness = compute_mean_smoothness();
    m_num_iterations = 0;
    m_state = OPTIMISING;
}

void
PoSyOptimiser::setup_termination_criteria() {
    using namespace std;
    m_termination_criteria = 0;
    const auto term_crit = m_properties.getProperty("posy-termination-criteria");
    stringstream ss(term_crit);
    while (ss.good()) {
        string option;
        getline(ss, option, ',');
        const auto old_term_crit = m_termination_criteria;
        if (option == "absolute" && ((m_termination_criteria & TC_ABSOLUTE) == 0)) {
            m_termination_criteria |= TC_ABSOLUTE;
            m_term_crit_absolute_smoothness = m_properties.getFloatProperty("posy-term-crit-absolute-smoothness");
        }
        if (option == "relative" && ((m_termination_criteria & TC_RELATIVE) == 0)) {
            m_termination_criteria |= TC_RELATIVE;
            m_term_crit_relative_smoothness = m_properties.getFloatProperty("posy-term-crit-relative-smoothness");
        }
        if (option == "fixed" && ((m_termination_criteria & TC_FIXED) == 0)) {
            m_termination_criteria |= TC_FIXED;
            m_term_crit_max_iterations = m_properties.getIntProperty("posy-term-crit-max-iterations");
        }
        // Warn if the flag was ignored.
        if (m_termination_criteria == old_term_crit) {
            spdlog::warn("Ignoring termination criteria {}", option);
        }
    }
}


/**
 * Perform post-smoothing tidy up.
 */
void
PoSyOptimiser::optimise_end() {
    assert(m_state == ENDING_OPTIMISATION);
    // TODO: Consider a final state here that can transition back to INITAILISED or make both READY
    m_state = INITIALISED;
}

void trace_smoothing(const SurfelGraphPtr &surfel_graph) {
    const auto old_level = spdlog::default_logger_raw()->level();
    spdlog::default_logger_raw()->set_level(spdlog::level::trace);
    spdlog::debug("Round completed.");
    for (const auto &n : surfel_graph->nodes()) {
        Eigen::Vector3f p, no, t, t1, v;
        if (n->data()->is_in_frame(0)) {
            n->data()->get_all_data_for_surfel_in_frame(0, v, t, t1, no, p);
            spdlog::trace("  p : ({:3f}, {:3f}, {:3f}),  d : ({:3f}, {:3f}),   s : {:3f}",
                          p[0], p[1], p[2],
                          n->data()->posy_correction()[0],
                          n->data()->posy_correction()[1],
                          n->data()->posy_smoothness()
            );
        }
    }
    spdlog::default_logger_raw()->set_level(old_level);
}

/**
 * Perform a single step of optimisation. Return true if converged or halted.
 */
bool
PoSyOptimiser::optimise_do_one_step() {
    assert(m_state != UNINITIALISED);

    if (m_state == INITIALISED) {
        optimise_begin();
    }

    if (m_state == OPTIMISING) {
        auto nodes_to_optimise = select_nodes_to_optimise();
        for (const auto &node : nodes_to_optimise) {
            optimise_node(node);
        }

        ++m_num_iterations;

        if (m_properties.getBooleanProperty("trace-smoothing")) {
            trace_smoothing(m_surfel_graph);
        }

        float smoothness;
        if (check_termination_criteria(smoothness, m_result)) {
            m_state = ENDING_OPTIMISATION;
        }
        m_last_smoothness = smoothness;
    }

    if (m_state == ENDING_OPTIMISATION) {
        optimise_end();
        return true;
    }
    return false;
}

std::vector<SurfelGraphNodePtr>
PoSyOptimiser::select_nodes_to_optimise() const {
    using namespace std;

    assert(m_node_selection_function);
    return m_node_selection_function(*this);
}

float
PoSyOptimiser::compute_node_smoothness_for_frame(const SurfelGraphNodePtr &node_ptr,
                                                 size_t frame_index,
                                                 unsigned int &num_neighbours) const {
    float frame_smoothness = 0.0f;

    Eigen::Vector3f vertex, reference_lattice_vertex, normal, tangent, orth_tangent;
    node_ptr->data()->get_all_data_for_surfel_in_frame(
            frame_index,
            vertex,
            tangent,
            orth_tangent,
            normal,
            reference_lattice_vertex
    );

    // For each neighbour...
    num_neighbours = 0;
    for (const auto &neighbour_node : m_surfel_graph->neighbours(node_ptr)) {
        if (!neighbour_node->data()->is_in_frame(frame_index)) {
            continue;
        }

        Eigen::Vector3f nbr_vertex, nbr_reference_lattice_vertex, nbr_normal, nbr_tangent, nbr_orth_tangent;
        neighbour_node->data()->get_all_data_for_surfel_in_frame(
                frame_index,
                nbr_vertex,
                nbr_tangent,
                nbr_orth_tangent,
                nbr_normal,
                nbr_reference_lattice_vertex);

        // Compute q_ij ... the midpoint on the intersection of the tangent planes
        const auto q_ij = compute_qij(vertex, normal, nbr_vertex, nbr_normal);
        const auto Q_ij = compute_lattice_neighbours(reference_lattice_vertex, q_ij, tangent, orth_tangent, m_rho);
        const auto Q_ji = compute_lattice_neighbours(nbr_reference_lattice_vertex, q_ij, nbr_tangent, nbr_orth_tangent,
                                                     m_rho);

        const auto closest_pair = find_closest_points(Q_ij, Q_ji);
        const auto cp_i = closest_pair.first;
        const auto cp_j = closest_pair.second;

        // Compute the smoothness over this surfel in this frame and the neighbour in this frame.
        const auto delta = (cp_j - cp_i).squaredNorm();
        if (m_properties.getBooleanProperty("diagnose_dodgy_deltas")) {
            if (delta > 0.866) {
                spdlog::warn("Unlikely looking closest distance {:4} for surfels {} and {}", delta,
                             node_ptr->data()->id(),
                             neighbour_node->data()->id());
                spdlog::warn("v_i = [{:3}, {:3}, {:3}]", vertex[0], vertex[1], vertex[2]);
                spdlog::warn("tan_i = [{:3}, {:3}, {:3}]", tangent[0], tangent[1], tangent[2]);
                spdlog::warn("tan_j = [{:3}, {:3}, {:3}]", nbr_tangent[0], nbr_tangent[1], nbr_tangent[2]);
                spdlog::warn("otan_i = [{:3}, {:3}, {:3}]", orth_tangent[0], orth_tangent[1], orth_tangent[2]);
                spdlog::warn("otan_j = [{:3}, {:3}, {:3}]", nbr_orth_tangent[0], nbr_orth_tangent[1],
                             nbr_orth_tangent[2]);
                spdlog::warn("v_j = [{:3}, {:3}, {:3}]", nbr_vertex[0], nbr_vertex[1], nbr_vertex[2]);
                spdlog::warn("q_ij = [{:3f}, {:3f}, {:3f}]", q_ij[0], q_ij[1], q_ij[2]);
                spdlog::warn("Qij = [{:3f}, {:3f}, {:3f};", Q_ij[0][0], Q_ij[0][1], Q_ij[0][2]);
                spdlog::warn("       {:3f}, {:3f}, {:3f};", Q_ij[1][0], Q_ij[1][1], Q_ij[1][2]);
                spdlog::warn("       {:3f}, {:3f}, {:3f};", Q_ij[3][0], Q_ij[3][1], Q_ij[3][2]);
                spdlog::warn("       {:3f}, {:3f}, {:3f};", Q_ij[2][0], Q_ij[2][1], Q_ij[2][2]);
                spdlog::warn("       {:3f}, {:3f}, {:3f}]", Q_ij[0][0], Q_ij[0][1], Q_ij[0][2]);
                spdlog::warn("Qji = [{:3f}, {:3f}, {:3f};", Q_ji[0][0], Q_ji[0][1], Q_ji[0][2]);
                spdlog::warn("       {:3f}, {:3f}, {:3f};", Q_ji[1][0], Q_ji[1][1], Q_ji[1][2]);
                spdlog::warn("       {:3f}, {:3f}, {:3f};", Q_ji[3][0], Q_ji[3][1], Q_ji[3][2]);
                spdlog::warn("       {:3f}, {:3f}, {:3f};", Q_ji[2][0], Q_ji[2][1], Q_ji[2][2]);
                spdlog::warn("       {:3f}, {:3f}, {:3f}]", Q_ji[0][0], Q_ji[0][1], Q_ji[0][2]);
                spdlog::warn("n_i = [{:3f}, {:3f}, {:3f}]", normal[0], normal[1], normal[2]);
                spdlog::warn("n_j = [{:3f}, {:3f}, {:3f}]", nbr_normal[0], nbr_normal[1], nbr_normal[2]);
                spdlog::warn("cl_i = [{:3f}, {:3f}, {:3f}]", cp_i[0], cp_i[1], cp_i[2]);
                spdlog::warn("cl_j = [{:3f}, {:3f}, {:3f}]", cp_j[0], cp_j[1], cp_j[2]);
                const auto axis = normal.cross(nbr_normal);

                // Do some checking
                spdlog::warn("n_i . (Q_i3 - Q_i1) is {:4}", normal.dot(Q_ij[3] - Q_ij[1]));
                spdlog::warn("n_j . (Q_j3 - Q_j1) is {:4}", normal.dot(Q_ji[3] - Q_ji[1]));
            }
        }
        frame_smoothness += delta;
        ++num_neighbours;
    }
    return frame_smoothness;
}

float
PoSyOptimiser::compute_mean_node_smoothness(const SurfelGraphNodePtr &node_ptr) const {
    float node_smoothness = 0.0f;
    unsigned int num_neighbours = 0;

    // For each frame in which this surfel appears
    unsigned int num_neighbours_in_frame;
    for (const auto &frame : node_ptr->data()->frames()) {
        // Compute the smoothness in this frame
        node_smoothness += compute_node_smoothness_for_frame(node_ptr, frame, num_neighbours_in_frame);
        num_neighbours += num_neighbours_in_frame;
    }
    // Set the mean smoothness
    const auto mean_smoothness = (num_neighbours == 0)
                                 ? 0
                                 : node_smoothness / (float) num_neighbours;

    node_ptr->data()->set_posy_smoothness(mean_smoothness);
    return mean_smoothness;
}

/**
 * Compute the mean smoothness for the graph.
 * Total smoothness across every node/frame/neighbour
 * divided by the total number of node * frame * neighbour
 */
float
PoSyOptimiser::compute_mean_smoothness() const {
    float total_smoothness = 0.0f;
    for (const auto &n : m_surfel_graph->nodes()) {
        total_smoothness += compute_mean_node_smoothness(n);
    }
    return total_smoothness / (float) m_surfel_graph->num_nodes();
}

bool
PoSyOptimiser::maybe_check_convergence(float &latest_smoothness, OptimisationResult &result) const {
    if ((m_termination_criteria & (TC_ABSOLUTE | TC_RELATIVE)) == 0) {
        return false;
    }

    latest_smoothness = compute_mean_smoothness();
    spdlog::info("Total smoothness: {}", latest_smoothness);
    // If it's 0 then we converged, regardless of whether we're checking for absolute smoothness or not.
    if (std::abs(latest_smoothness) < 1e-9) {
        result = CONVERGED;
        return true;
    }

    if ((m_termination_criteria & TC_RELATIVE) != 0) {
        float improvement = m_last_smoothness - latest_smoothness;
        float pct = (100.0f * improvement) / m_last_smoothness;
        spdlog::info("  Improvement {}%", pct);
        if ((pct >= 0) && (std::abs(pct) < m_term_crit_relative_smoothness)) {
            result = CONVERGED;
            return true;
        }
    }

    if ((m_termination_criteria & TC_ABSOLUTE) != 0) {
        if (latest_smoothness <= m_term_crit_absolute_smoothness) {
            result = CONVERGED;
            return true;
        }
    }
    return false;
}


bool
PoSyOptimiser::maybe_check_iterations(OptimisationResult &result) const {
    if ((m_termination_criteria & TC_FIXED) == 0) {
        return false;
    }

    if (m_num_iterations >= m_term_crit_max_iterations) {
        result = CONVERGED;
        return true;
    }
    return false;
}

/**
 * Check whether user asked for optimising to halt.
 */
bool
PoSyOptimiser::user_canceled_optimise() {
    struct stat buffer{};
    return (stat("halt", &buffer) == 0);
}

bool
PoSyOptimiser::check_cancellation(OptimisationResult &result) {
    if (!user_canceled_optimise()) {
        return false;
    }

    result = CANCELLED;
    return true;
}

/**
 * Measure the change in error. If it's below some threshold, consider this level converged.
 */
bool
PoSyOptimiser::check_termination_criteria(
        float &smoothness,
        OptimisationResult &result) const {

    if (check_cancellation(result))
        return true;

    if (maybe_check_convergence(smoothness, result))
        return true;

    if (maybe_check_iterations(result))
        return true;

    return false;
}

/**
 * @param surfel_graph The graph.
 * @return the number of frames spanned by this graph.
 */
unsigned int
PoSyOptimiser::count_number_of_frames(const SurfelGraphPtr &surfel_graph) {
    // Compute the number of frames
    unsigned int max_frame_id = 0;
    for (const auto &n : surfel_graph->nodes()) {
        for (const auto &fd : n->data()->frame_data()) {
            if (fd.pixel_in_frame.frame > max_frame_id) {
                max_frame_id = fd.pixel_in_frame.frame;
            }
        }
    }
    return max_frame_id + 1;
}

void
PoSyOptimiser::set_data(const SurfelGraphPtr &surfel_graph) {
    m_surfel_graph = surfel_graph;
    m_state = INITIALISED;
    m_num_frames = count_number_of_frames(surfel_graph);
}

std::function<std::vector<SurfelGraphNodePtr>(const PoSyOptimiser &)>
PoSyOptimiser::extractSsa(const Properties &properties) {
    auto ssa = with_name(properties.getProperty("surfel-selection-algorithm"));
    switch (ssa) {
        case SSA_SELECT_ALL_IN_RANDOM_ORDER:
            return std::bind(&PoSyOptimiser::ssa_select_all_in_random_order, this);

        case SSA_SELECT_WORST_100:
            return std::bind(&PoSyOptimiser::ssa_select_worst_100, this);
    }
}

/**
 * Surfel selection model 2: Select worst 100 smoothness scores
 */
std::vector<SurfelGraphNodePtr>
PoSyOptimiser::ssa_select_worst_100() {
    using namespace std;

    vector<SurfelGraphNodePtr> selected_nodes;
    for (const auto &n : m_surfel_graph->nodes()) {
        selected_nodes.push_back(n);
    }
    stable_sort(begin(selected_nodes), end(selected_nodes),
                [](const SurfelGraphNodePtr &s1, const SurfelGraphNodePtr &s2) {
                    return s1->data()->posy_smoothness() > s2->data()->posy_smoothness();
                });

    selected_nodes.resize(100);
    return selected_nodes;
}

/**
 * Select all surfels in a layer and randomize the order
 */
std::vector<SurfelGraphNodePtr>
PoSyOptimiser::ssa_select_all_in_random_order() {
    using namespace std;

    vector<size_t> indices(m_surfel_graph->num_nodes());
    iota(begin(indices), end(indices), 0);
    shuffle(begin(indices), end(indices), default_random_engine(123));
    vector<SurfelGraphNodePtr> selected_nodes;
    selected_nodes.reserve(indices.size());
    const auto graph_nodes = m_surfel_graph->nodes();
    for (const auto i : indices) {
        selected_nodes.push_back(graph_nodes.at(i));
    }
    return selected_nodes;
}

Eigen::Vector2f compute_ref_offset(
        const Eigen::Vector3f &new_lattice_point,
        const Eigen::Vector3f &vertex,
        const Eigen::Vector3f &tangent,
        const Eigen::Vector3f &orth_tangent) {
    const auto diff = new_lattice_point - vertex;
    return {
            diff.dot(tangent),
            diff.dot(orth_tangent)
    };
}


/**
 * Optimise this GraphNode by considering all neighbours and allowing them all to
 * 'push' this node slightly to an agreed common position.
 * @param node
 */
void
PoSyOptimiser::optimise_node(const SurfelGraphNodePtr &node) {
    using namespace Eigen;

    const auto &this_surfel_ptr = node->data();

    // For each frame this surfel is in...
    for (const auto frame_index : this_surfel_ptr->frames()) {
        Eigen::Vector3f vertex, reference_lattice_point, normal, tangent, orth_tangent;
        this_surfel_ptr->get_all_data_for_surfel_in_frame(
                frame_index,
                vertex,
                tangent,
                orth_tangent,
                normal,
                reference_lattice_point);

        float sum_w = 0.0f;
        Vector3f new_lattice_point = reference_lattice_point;

        // For each neighbour j ...
        for (const auto &neighbour_node : m_surfel_graph->neighbours(node)) {
            if (!neighbour_node->data()->is_in_frame(frame_index)) {
                continue;
            }
            Eigen::Vector3f nbr_vertex, nbr_lattice_point, nbr_normal, nbr_tangent, nbr_orth_tangent;
            neighbour_node->data()->get_all_data_for_surfel_in_frame(
                    frame_index,
                    nbr_vertex,
                    nbr_tangent,
                    nbr_orth_tangent,
                    nbr_normal,
                    nbr_lattice_point);

            float w_ij = 1.0f;

            // Compute q_ij and thus Q_ij and Q_ji
            const auto q_ij = compute_qij(vertex, normal, nbr_vertex, nbr_normal);
            const auto Q_ij = compute_lattice_neighbours(new_lattice_point, q_ij, tangent, orth_tangent, m_rho);
            const auto Q_ji = compute_lattice_neighbours(nbr_lattice_point, q_ij, nbr_tangent, nbr_orth_tangent, m_rho);
            const auto closest_points = find_closest_points(Q_ij, Q_ji);

            new_lattice_point = sum_w * closest_points.first + w_ij * closest_points.second;
            sum_w += w_ij;
            new_lattice_point = new_lattice_point * (1.0f / sum_w);
            new_lattice_point -= normal.dot(new_lattice_point - vertex) * normal; // Back to plane
        } // End of neighbours
        new_lattice_point = round_4(normal, tangent, new_lattice_point, vertex, m_rho);
        const auto ref_offset = compute_ref_offset(new_lattice_point, vertex, tangent, orth_tangent);
        this_surfel_ptr->set_reference_lattice_offset(ref_offset);
    } // End of frames
}