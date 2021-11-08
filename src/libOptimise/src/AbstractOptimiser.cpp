//
// Created by Dave Durbin (Old) on 4/5/21.
//

#include "AbstractOptimiser.h"

#include <random>
#include <sstream>
#include <utility>
#include <algorithm>    // random_shuffle
#include <sys/stat.h>
#include <spdlog/spdlog.h>
#include <Properties/Properties.h>       // termination criteria set up

AbstractOptimiser::AbstractOptimiser(Properties properties, std::mt19937& rng)
    : Optimiser{properties, rng}//
    , m_result{NOT_COMPLETE} //
    , m_state{UNINITIALISED} //
    , m_randomise_neighour_order{false} //
    , m_termination_criteria{0} //
    , m_term_crit_absolute_smoothness{0.0f} //
    , m_term_crit_relative_smoothness{0.0f} //
    , m_term_crit_max_iterations{0} //
    , m_num_iterations{0} //
    , m_num_frames{0} //
    , m_last_smoothness{std::numeric_limits<float>::infinity()} //
{

}

unsigned short
AbstractOptimiser::read_termination_criteria(const std::string &termination_criteria_property) {
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
AbstractOptimiser::setup_termination_criteria(
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
AbstractOptimiser::optimise_begin() {
  assert(m_state == INITIALISED);

  using namespace spdlog;
  info("Computing initial smoothness");
  m_last_smoothness = compute_mean_smoothness();
  info("Initial smoothness : {:4.3f}", m_last_smoothness);
  m_num_iterations = 0;
  m_state = OPTIMISING;
}

/**
 * Check whether user asked for optimising to halt.
 */
bool
AbstractOptimiser::user_canceled_optimise() {
  struct stat buffer{};
  return (stat("halt", &buffer) == 0);
}

bool
AbstractOptimiser::check_cancellation(OptimisationResult &result) {
  if (!user_canceled_optimise()) {
    return false;
  }
  spdlog::info("Terminating because of cancellation");
  result = CANCELLED;
  return true;
}

bool
AbstractOptimiser::maybe_check_convergence(float &latest_smoothness, OptimisationResult &result) const {
  // Always bail if converged to 0
  if (m_last_smoothness == 0) {
    spdlog::info("Terminating because m_last_smoothness is 0");
    result = CONVERGED;
    return true;
  }

  latest_smoothness = compute_mean_smoothness();
  // Otherwise return early if we're not checking for convergence
  if ((m_termination_criteria & (TC_ABSOLUTE | TC_RELATIVE)) == 0) {
    return false;
  }

  float improvement = m_last_smoothness - latest_smoothness;
  float pct = (100.0f * improvement) / m_last_smoothness;
  spdlog::info("Mean smoothness per node: {}, Improvement {}%", latest_smoothness, pct);
  // If it's 0 then we converged, regardless of whether we're checking for absolute smoothness or not.
  if (std::abs(latest_smoothness) < 1e-9) {
    spdlog::info("Terminating because latest_smoothness is practically 0");
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
AbstractOptimiser::maybe_check_iterations(OptimisationResult &result) const {
  if ((m_termination_criteria & TC_FIXED) == 0) {
    return false;
  }
  float latest_smoothness = compute_mean_smoothness();
  spdlog::info("Mean smoothness per node: {}", latest_smoothness);
  if (m_num_iterations >= m_term_crit_max_iterations) {
    spdlog::info("Terminating because we completed the specified number of iterations");
    result = CONVERGED;
    return true;
  }
  return false;
}

/**
 * Measure the change in error. If it's below some threshold, consider this level converged.
 */
bool
AbstractOptimiser::check_termination_criteria(
    float &smoothness,
    OptimisationResult &result) const {

  if (check_cancellation(result)) {
    return true;
  }

  if (maybe_check_convergence(smoothness, result)) {
    return true;
  }

  if (maybe_check_iterations(result)) {
    return true;
  }

  return false;
}

void
AbstractOptimiser::optimise_end() {
  assert(m_state == ENDING_OPTIMISATION);
  // TODO: Consider a final state here that can transition back to INITIALISED or make both READY
  m_state = INITIALISED;
}

bool
AbstractOptimiser::optimise_do_one_step() {
  assert(m_state != UNINITIALISED);

  if (m_state == INITIALISED) {
    optimise_begin();
  }

  if (m_state == OPTIMISING) {
    optimise_do_pass();
    ++m_num_iterations;

    if (m_properties.getBooleanProperty("trace-smoothing")) {
      trace_smoothing(m_surfel_graph);
    }

    float smoothness = std::numeric_limits<float>::infinity();
    OptimisationResult result;
    if (check_termination_criteria(smoothness, result)) {
      m_state = ENDING_OPTIMISATION;
      m_result = result;
      smoothing_completed(smoothness, result);
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
AbstractOptimiser::compute_mean_node_smoothness(const SurfelGraphNodePtr &node_ptr) const {
  float node_smoothness = 0.0f;
  unsigned int num_neighbours = 0;

  // For each frame in which this surfel appears
  unsigned int num_neighbours_in_frame;
  for (const auto &frame: node_ptr->data()->frames()) {
    // Compute the smoothness in this frame
    node_smoothness += compute_node_smoothness_for_frame(node_ptr, frame, num_neighbours_in_frame);
    num_neighbours += num_neighbours_in_frame;
  }
  // Set the mean smoothness
  const auto mean_smoothness = (num_neighbours == 0)
                               ? 0
                               : node_smoothness / (float) num_neighbours;

  spdlog::debug(" mean_smoothness {:4.3f}", mean_smoothness);
  return mean_smoothness;
}

float
AbstractOptimiser::compute_mean_smoothness() const {
  float total_smoothness = 0.0f;
  for (const auto &n: m_surfel_graph->nodes()) {
    auto mean_smoothness = compute_mean_node_smoothness(n);
    total_smoothness += mean_smoothness;
    store_mean_smoothness(n, mean_smoothness);
  }
  return total_smoothness / (float) m_surfel_graph->num_nodes();
}


void
AbstractOptimiser::set_data(const SurfelGraphPtr &surfel_graph) {
  m_surfel_graph = surfel_graph;
  m_num_frames = get_num_frames(surfel_graph);
  m_state = INITIALISED;
  loaded_graph();
}

std::vector<SurfelGraphNodePtr>
AbstractOptimiser::get_neighbours_of_node_in_frame(
    const SurfelGraphPtr &graph,
    const SurfelGraphNodePtr &node_ptr,
    unsigned int frame_index,
    bool randomise_order) const {

  auto neighbours_in_frame = get_node_neighbours_in_frame(m_surfel_graph, node_ptr, frame_index);
  // Optionally randomise the order
  if (randomise_order) {
    std::shuffle(begin(neighbours_in_frame),
                 end(neighbours_in_frame),
                 m_random_engine);
  }
  return neighbours_in_frame;
}

/* Call back when termination criteria are met */
void AbstractOptimiser::smoothing_completed(float smoothness, OptimisationResult result) {
  spdlog::info("Terminating because {}. Smoothness : {:4.3f}",
               m_result == NOT_COMPLETE
               ? "not complete"
               : m_result == CONVERGED
                 ? "converged"
                 : "cancelled", smoothness
  );
}
