#include "RoSy/RoSyOptimiser.h"

#include <RoSy/RoSy.h>
#include <Optimise/SurfelSelectionAlgorithm.h>
#include <Properties/Properties.h>
#include <Surfel/Surfel.h>
#include <Surfel/Surfel_IO.h>

#include <Eigen/Core>

#include <random>
#include <iostream>
#include <utility>
#include <sys/stat.h>
#include <spdlog/spdlog.h>

/**
 * Construct one with the given properties.
 * @param properties
 */
RoSyOptimiser::RoSyOptimiser(Properties properties)
        : m_termination_criteria{0} //
        , m_term_crit_absolute_smoothness{0.0f} //
        , m_term_crit_relative_smoothness{0.0f} //
        , m_term_crit_max_iterations{0} //
        , m_result{NOT_COMPLETE} //
        , m_last_smoothness{0.0f} //
        , m_num_iterations{0} //
        , m_num_frames{0} //
        , m_properties{std::move(properties)} //
        , m_random_engine{123} //
        , m_state{UNINITIALISED} //
{
    m_surfels_per_step = m_properties.getIntProperty("rosy-surfels-per-step");
    assert(m_surfels_per_step > 0);

    m_node_selection_function = extractSsa(m_properties);

    setup_termination_criteria();
}

void
RoSyOptimiser::setup_termination_criteria() {
    using namespace std;
    m_termination_criteria = 0;
    const auto term_crit = m_properties.getProperty("rosy-termination-criteria");
    stringstream ss(term_crit);
    while (ss.good()) {
        string option;
        getline(ss, option, ',');
        const auto old_term_crit = m_termination_criteria;
        if (option == "absolute" && ((m_termination_criteria & TC_ABSOLUTE) == 0)) {
            m_termination_criteria |= TC_ABSOLUTE;
            m_term_crit_absolute_smoothness = m_properties.getFloatProperty("rosy-term-crit-absolute-smoothness");
        }
        if (option == "relative" && ((m_termination_criteria & TC_RELATIVE) == 0)) {
            m_termination_criteria |= TC_RELATIVE;
            m_term_crit_relative_smoothness = m_properties.getFloatProperty("rosy-term-crit-relative-smoothness");
        }
        if (option == "fixed" && ((m_termination_criteria & TC_FIXED) == 0)) {
            m_termination_criteria |= TC_FIXED;
            m_term_crit_max_iterations = m_properties.getIntProperty("rosy-term-crit-max-iterations");
        }
        // Warn if the flag was ignored.
        if (m_termination_criteria == old_term_crit) {
            spdlog::warn("Ignoring termination criteria {}", option);
        }
    }
}


std::function<std::vector<SurfelGraphNodePtr>(const RoSyOptimiser &)>
RoSyOptimiser::extractSsa(const Properties &properties) {
    auto ssa = with_name(properties.getProperty("rosy-surfel-selection-algorithm"));
    switch (ssa) {
        case SSA_SELECT_ALL_IN_RANDOM_ORDER:
            return std::bind(&RoSyOptimiser::ssa_select_all_in_random_order, this);

        case SSA_SELECT_WORST_100:
            return std::bind(&RoSyOptimiser::ssa_select_worst_100, this);
    }
}


/**
 * Check whether user asked for optimising to halt.
 */
bool
RoSyOptimiser::user_canceled_optimise() {
    struct stat buffer{};
    return (stat("halt", &buffer) == 0);
}

/**
 * Return a const reference to the surfels being transformed so externals can play with it
 */
std::vector<std::shared_ptr<Surfel>>
RoSyOptimiser::get_surfel_data() const {
    using namespace std;
    vector<shared_ptr<Surfel>> surfels;
    for (const auto &n : m_surfel_graph->nodes()) {
        surfels.push_back(n->data());
    }
    return surfels;
}

/**
 * @param surfel_graph The graph.
 * @return the number of frames spanned by this graph.
 */
unsigned int
RoSyOptimiser::count_number_of_frames(const SurfelGraphPtr &surfel_graph) {
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
RoSyOptimiser::set_data(SurfelGraphPtr &surfel_graph) {
    m_surfel_graph = surfel_graph;
    m_num_frames = count_number_of_frames(surfel_graph);
    m_state = INITIALISED;
}

/**
 * Start global smoothing.
 */
void
RoSyOptimiser::optimise_begin() {
    assert(m_state == INITIALISED);

    using namespace std;
    using namespace spdlog;

    m_last_smoothness = compute_mean_smoothness();
    m_num_iterations = 0;
    m_state = OPTIMISING;
}

/**
 * Perform post-smoothing tidy up.
 */
void
RoSyOptimiser::optimise_end() {
    assert(m_state == ENDING_OPTIMISATION);

    // TODO: Consider a final state here that can transition back to INITAILISED or make both READY
    m_state = INITIALISED;
}

bool
RoSyOptimiser::maybe_check_convergence(float &latest_smoothness, OptimisationResult &result) const {
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
RoSyOptimiser::maybe_check_iterations(OptimisationResult &result) const {
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
 * Measure the change in error. If it's below some threshold, consider this level converged.
 */
bool
RoSyOptimiser::check_termination_criteria(
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

std::vector<SurfelGraphNodePtr>
RoSyOptimiser::node_neighbours_in_frame(const SurfelGraphNodePtr &node_ptr, unsigned int frame_index) const {
    std::vector<SurfelGraphNodePtr> neighbours_in_frame;
    for (const auto &neighbour_node : m_surfel_graph->neighbours(node_ptr)) {
        if (neighbour_node->data()->is_in_frame(frame_index)) {
            neighbours_in_frame.emplace_back(neighbour_node);
        }
    }
    return neighbours_in_frame;
}


/**
 * @return The mean error per neighbour.
 */
float
RoSyOptimiser::compute_node_smoothness_for_frame(
        const SurfelGraphNodePtr &node_ptr,
        size_t frame_index,
        unsigned int& num_neighbours) const {

    float frame_smoothness = 0.0f;

    Eigen::Vector3f vertex, normal, tangent;
    node_ptr->data()->get_vertex_tangent_normal_for_frame(frame_index, vertex, tangent, normal);

    const auto neighbours_in_frame = node_neighbours_in_frame(node_ptr, frame_index);

    // For each neighbour in frame...
    for (const auto &neighbour_node : neighbours_in_frame) {
        Eigen::Vector3f nbr_vertex, nbr_normal, nbr_tangent;
        neighbour_node->data()->get_vertex_tangent_normal_for_frame(frame_index, nbr_vertex, nbr_tangent, nbr_normal);

        // Compute the error between this surfel in this frame and the neighbour in this frame.
        auto vec_pair = best_rosy_vector_pair(tangent, normal, nbr_tangent, normal);
        float theta = degrees_angle_between_vectors(vec_pair.first, vec_pair.second);
        frame_smoothness += (theta * theta);
    }

    num_neighbours = neighbours_in_frame.size();
    return frame_smoothness;
}

float
RoSyOptimiser::compute_mean_node_smoothness(const SurfelGraphNodePtr &node_ptr) const {
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
    const auto mean_smoothness = (num_neighbours == 0 )
            ? 0
            : node_smoothness / (float)num_neighbours;

    node_ptr->data()->set_rosy_smoothness(mean_smoothness);
    return mean_smoothness;
}

/**
 * Compute the mean smoothness for the graph.
 * Total smoothness across every node/frame/neighbour
 * divided by the total number of node * frame * neighbour
 */
float
RoSyOptimiser::compute_mean_smoothness() const {
    float total_smoothness = 0.0f;
    for (const auto &n : m_surfel_graph->nodes()) {
        total_smoothness += compute_mean_node_smoothness(n);
    }
    return total_smoothness / (float)m_surfel_graph->num_nodes();
}

bool
RoSyOptimiser::check_cancellation(OptimisationResult &result) {
    if (!user_canceled_optimise()) {
        return false;
    }

    result = CANCELLED;
    return true;
}

/**
 * Select all surfels in a layer and randomize the order
 */
std::vector<SurfelGraphNodePtr>
RoSyOptimiser::ssa_select_all_in_random_order() {
    using namespace std;

    vector<size_t> indices(m_surfel_graph->num_nodes());
    iota(begin(indices), end(indices), 0);
    shuffle(begin(indices), end(indices), m_random_engine);
    vector<SurfelGraphNodePtr> selected_nodes;
    selected_nodes.reserve(indices.size());
    const auto graph_nodes = m_surfel_graph->nodes();
    for (const auto i : indices) {
        selected_nodes.push_back(graph_nodes.at(i));
    }
    return selected_nodes;
}

/**
 * Surfel selection model 2: Select top 100 error scores
 */
std::vector<SurfelGraphNodePtr>
RoSyOptimiser::ssa_select_worst_100() {
    using namespace std;

    vector<SurfelGraphNodePtr> selected_nodes;
    for (const auto &n : m_surfel_graph->nodes()) {
        selected_nodes.push_back(n);
    }
    stable_sort(begin(selected_nodes), end(selected_nodes),
                [](const SurfelGraphNodePtr &s1, const SurfelGraphNodePtr &s2) {
                    return s1->data()->rosy_smoothness() > s2->data()->rosy_smoothness();
                });

    selected_nodes.resize(100);
    return selected_nodes;
}

std::vector<SurfelGraphNodePtr>
RoSyOptimiser::select_nodes_to_optimise() {
    using namespace std;

    assert(m_node_selection_function);
    return m_node_selection_function(*this);
}

/**
 * Perform a single step of optimisation.
 */
bool
RoSyOptimiser::optimise_do_one_step() {
    assert(m_state != UNINITIALISED);

    using namespace std;

    if (m_state == INITIALISED) {
        optimise_begin();
    }

    if (m_state == OPTIMISING) {
        auto nodes_to_optimise = select_nodes_to_optimise();
        for (const auto &node : nodes_to_optimise) {
            optimise_node(node);
        }

        ++m_num_iterations;

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

void
RoSyOptimiser::optimise_node(const SurfelGraphNodePtr &node) {
    using namespace Eigen;

    const auto &this_surfel_ptr = node->data();
    const auto &old_tangent = this_surfel_ptr->tangent();
    Vector3f new_tangent{old_tangent};
    float weight = 0;

    // For each frame that this surfel appears in.
    for (const auto frame_index : this_surfel_ptr->frames()) {
        Eigen::Vector3f vertex, reference_lattice_point, normal, tangent, orth_tangent;
        this_surfel_ptr->get_all_data_for_surfel_in_frame(
                frame_index,
                vertex,
                tangent,
                orth_tangent,
                normal,
                reference_lattice_point);


        // For each neighbour j in frame ...
        const auto neighbours_in_frame = node_neighbours_in_frame(node, frame_index);
        for (const auto &neighbour_node : neighbours_in_frame) {
            const auto &that_surfel_ptr = neighbour_node->data();
            auto edge = m_surfel_graph->edge(node, neighbour_node);
            const auto w_ij = 1.0f;

            Vector3f neighbour_tan_in_surfel_space, neighbour_norm_in_surfel_space;
            this_surfel_ptr->transform_surfel_via_frame(that_surfel_ptr, frame_index,
                                                        neighbour_norm_in_surfel_space,
                                                        neighbour_tan_in_surfel_space);

            unsigned short source_k;
            unsigned short target_k;
            new_tangent = average_rosy_vectors(
                    new_tangent,
                    Vector3f::UnitY(),
                    weight,
                    neighbour_tan_in_surfel_space,
                    neighbour_norm_in_surfel_space,
                    w_ij,
                    target_k,
                    source_k);

            weight += w_ij;

            // Store ks
            edge->set_k_ij(frame_index, target_k);
            edge->set_k_ji(frame_index, source_k);
        }
    }

    node->data()->setTangent(new_tangent);
    auto vec_pair = best_rosy_vector_pair(new_tangent, Vector3f::UnitY(), old_tangent, Vector3f::UnitY());
    node->data()->set_rosy_correction(fmod(degrees_angle_between_vectors(vec_pair.first, vec_pair.second), 90.0f));
}
