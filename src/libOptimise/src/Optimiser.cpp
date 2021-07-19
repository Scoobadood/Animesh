//
// Created by Dave Durbin (Old) on 4/5/21.
//

#include "Optimiser.h"

#include <random>
#include <sstream>
#include <utility>
#include <numeric>      // iota
#include <random>       // default_random_engine initialisation
#include <algorithm>    // random_shuffle
#include <sys/stat.h>
#include <spdlog/spdlog.h>
#include <SurfelSelectionAlgorithm.h>
#include <Properties/Properties.h>       // termination criteria set up

Optimiser::Optimiser(Properties properties)
        : m_properties(std::move(properties)) //
        , m_termination_criteria{0} //
        , m_term_crit_absolute_smoothness{0.0f} //
        , m_term_crit_relative_smoothness{0.0f} //
        , m_term_crit_max_iterations{0} //
        , m_node_selection_function{nullptr} //
        , m_result{NOT_COMPLETE} //
        , m_num_iterations{0} //
        , m_num_frames{0} //
        , m_last_smoothness{0.0f} //
        , m_state{UNINITIALISED} //
{}

unsigned short
Optimiser::read_termination_criteria(const std::string &termination_criteria_property) {
    using namespace std;

    auto termination_criteria = m_properties.getProperty(termination_criteria_property);
    unsigned short criteria = 0;
    stringstream ss(termination_criteria);
    while (ss.good()) {
        string option;
        getline(ss, option, ',');
        const auto previous_criteria = criteria;
        if (option == "absolute" && ((criteria & TC_ABSOLUTE) == 0)) {
            criteria |= TC_ABSOLUTE;
        }
        if (option == "relative" && ((criteria & TC_RELATIVE) == 0)) {
            criteria |= TC_RELATIVE;
        }
        if (option == "fixed" && ((criteria & TC_FIXED) == 0)) {
            criteria |= TC_FIXED;
        }
        // Warn if the flag was ignored.
        if (criteria == previous_criteria) {
            spdlog::warn("Ignoring termination criteria {}", option);
        }
    }
    return criteria;
}

void
Optimiser::setup_termination_criteria(
        const std::string &termination_criteria_property,
        const std::string &relative_smoothness_property,
        const std::string &absolute_smoothness_property,
        const std::string &max_iterations_property) {
    using namespace std;

    m_termination_criteria = read_termination_criteria(termination_criteria_property);
    if ((m_termination_criteria & TC_ABSOLUTE) == TC_ABSOLUTE) {
        m_term_crit_absolute_smoothness = m_properties.getFloatProperty(absolute_smoothness_property);
    }
    if ((m_termination_criteria & TC_RELATIVE) == TC_RELATIVE) {
        m_term_crit_relative_smoothness = m_properties.getFloatProperty(relative_smoothness_property);
    }
    if ((m_termination_criteria & TC_FIXED) == TC_FIXED) {
        m_term_crit_max_iterations = m_properties.getIntProperty(max_iterations_property);
    }
}

void
Optimiser::setup_ssa() {
    m_node_selection_function = nullptr;
    auto ssa = with_name(m_properties.getProperty(get_ssa_property_name()));
    switch (ssa) {
        case SSA_SELECT_ALL_IN_RANDOM_ORDER:
            m_node_selection_function = std::bind(&Optimiser::ssa_select_all_in_random_order, this);
            break;

        case SSA_SELECT_WORST_100:
            m_node_selection_function = std::bind(&Optimiser::ssa_select_worst_100, this);
            break;

        case SSA_SELECT_WORST_PERCENTAGE:
            auto ssa_percentage = m_properties.getIntProperty(get_ssa_percentage_property_name());
            m_ssa_percentage = std::min<unsigned int>(std::max<unsigned int>(0, ssa_percentage), 100);
            m_node_selection_function = std::bind(&Optimiser::ssa_select_worst_percentage, this);
            break;
    }
}

void
Optimiser::optimise_begin() {
    assert(m_state == INITIALISED);

    using namespace spdlog;
    setup_ssa();
    info("Initialising error value");
    m_last_smoothness = compute_mean_smoothness();
    m_num_iterations = 0;
    m_state = OPTIMISING;
}

/**
 * Check whether user asked for optimising to halt.
 */
bool
Optimiser::user_canceled_optimise() {
    struct stat buffer{};
    return (stat("halt", &buffer) == 0);
}

bool
Optimiser::check_cancellation(OptimisationResult &result) {
    if (!user_canceled_optimise()) {
        return false;
    }

    result = CANCELLED;
    return true;
}

bool
Optimiser::maybe_check_convergence(float &latest_smoothness, OptimisationResult &result) const {
    if ((m_termination_criteria & (TC_ABSOLUTE | TC_RELATIVE)) == 0) {
        return false;
    }

    latest_smoothness = compute_mean_smoothness();
    float improvement = m_last_smoothness - latest_smoothness;
    float pct = (100.0f * improvement) / m_last_smoothness;
    spdlog::info("Mean smoothness per node: {}, Improvement {}%", latest_smoothness, pct);
    // If it's 0 then we converged, regardless of whether we're checking for absolute smoothness or not.
    if (std::abs(latest_smoothness) < 1e-9) {
        result = CONVERGED;
        return true;
    }

    if ((m_termination_criteria & TC_RELATIVE) != 0) {
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
Optimiser::maybe_check_iterations(OptimisationResult &result) const {
    if ((m_termination_criteria & TC_FIXED) == 0) {
        return false;
    }
    float latest_smoothness = compute_mean_smoothness();
    spdlog::info("Mean smoothness per node: {}", latest_smoothness);
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
Optimiser::check_termination_criteria(
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

void
Optimiser::optimise_end() {
    assert(m_state == ENDING_OPTIMISATION);
    // TODO: Consider a final state here that can transition back to INITAILISED or make both READY
    m_state = INITIALISED;
}

bool
Optimiser::optimise_do_one_step() {
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

float
Optimiser::compute_mean_node_smoothness(const SurfelGraphNodePtr &node_ptr) const {
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

    node_ptr->data()->set_rosy_smoothness(mean_smoothness);
    return mean_smoothness;
}

float
Optimiser::compute_mean_smoothness() const {
    float total_smoothness = 0.0f;
    for (const auto &n : m_surfel_graph->nodes()) {
        total_smoothness += compute_mean_node_smoothness(n);
    }
    return total_smoothness / (float) m_surfel_graph->num_nodes();
}

std::vector<SurfelGraphNodePtr>
Optimiser::select_nodes_to_optimise() {
    assert(m_node_selection_function);
    return m_node_selection_function(*this);
}

/**
 * Select all surfels and randomize the order
 */
std::vector<SurfelGraphNodePtr>
Optimiser::ssa_select_all_in_random_order() {
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
 * Surfel selection model 2: Select least smooth 100
 */
std::vector<SurfelGraphNodePtr>
Optimiser::ssa_select_worst_100() const {
    using namespace std;

    vector<SurfelGraphNodePtr> selected_nodes;
    for (const auto &n : m_surfel_graph->nodes()) {
        selected_nodes.push_back(n);
    }
    // Sort worst to best
    stable_sort(begin(selected_nodes),
                end(selected_nodes),
                [this](const SurfelGraphNodePtr &l, const SurfelGraphNodePtr &r) {
                    return compare_worst_first(l, r);
                });

    if (selected_nodes.size() > 100) {
        selected_nodes.resize(100);
    }
    return selected_nodes;
}

std::vector<SurfelGraphNodePtr>
Optimiser::ssa_select_worst_percentage() const {
    using namespace std;

    vector<SurfelGraphNodePtr> selected_nodes;
    for (const auto &n : m_surfel_graph->nodes()) {
        selected_nodes.push_back(n);
    }
    // Sort worst to best
    stable_sort(begin(selected_nodes),
                end(selected_nodes),
                [this](const SurfelGraphNodePtr &l, const SurfelGraphNodePtr &r) {
                    return compare_worst_first(l, r);
                });

    // Required nodes
    auto required_nodes = (unsigned int) std::roundf(((float)(m_surfel_graph->num_nodes()) * m_ssa_percentage) / 100);
    if (selected_nodes.size() > required_nodes) {
        selected_nodes.resize(required_nodes);
    }
    return selected_nodes;
}

void
Optimiser::set_data(const SurfelGraphPtr &surfel_graph) {
    m_surfel_graph = surfel_graph;
    m_num_frames = get_num_frames(surfel_graph);
    m_state = INITIALISED;
    loaded_graph();
}