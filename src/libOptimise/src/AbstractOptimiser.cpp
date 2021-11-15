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

AbstractOptimiser::AbstractOptimiser(Properties properties, std::default_random_engine &rng)
    : Optimiser{std::move(properties), rng}//
    , m_result{NOT_COMPLETE} //
    , m_state{UNINITIALISED} //
    , m_termination_criteria{0} //
    , m_term_crit_absolute_smoothness{0.0f} //
    , m_term_crit_relative_smoothness{0.0f} //
    , m_term_crit_max_iterations{0} //
    , m_num_iterations{0} //
    , m_num_frames{0} //
    , m_smoothness_is_current{false} //
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
  compute_smoothness(m_last_smoothness, m_frame_smoothness);
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
AbstractOptimiser::maybe_check_convergence(float &latest_smoothness, OptimisationResult &result) {
  // Always bail if converged to 0, even if we're running fixed iterations, no need to do extra work.
  if (m_last_smoothness == 0) {
    spdlog::info("Terminating because m_last_smoothness is 0");
    result = CONVERGED;
    return true;
  }

  // Otherwise return early if we're not checking for convergence
  if ((m_termination_criteria & (TC_ABSOLUTE | TC_RELATIVE)) == 0) {
    return false;
  }

  std::vector<float> frame_smoothness;
  frame_smoothness.resize(m_num_frames,0);
  float mean_node_smoothness;
  compute_smoothness(mean_node_smoothness, frame_smoothness);

  float improvement = m_last_smoothness - mean_node_smoothness;
  float pct = (100.0f * improvement) / m_last_smoothness;
  spdlog::info("Mean smoothness per node: {}, Improvement {}%", latest_smoothness, pct);

  m_last_smoothness = mean_node_smoothness;
  for (auto i = 0; i < m_num_frames; ++i) {
    m_frame_smoothness[i] = frame_smoothness[i];
  }
  m_smoothness_is_current = true;

  // If m_last_smoothness is 0 then we converged, regardless of whether we're checking for absolute smoothness or not.
  if (std::abs(m_last_smoothness) < 1e-9) {
    spdlog::info("Terminating because smoothness is practically 0");
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
  if (m_num_iterations >= m_term_crit_max_iterations) {
    spdlog::info("Terminating because we completed {} of iterations", m_num_iterations);
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
    OptimisationResult &result) {

  if (check_cancellation(result)) {
    return true;
  }

  if (maybe_check_iterations(result)) {
    return true;
  }

  if (maybe_check_convergence(smoothness, result)) {
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

void
AbstractOptimiser::compute_smoothness(
    float &mean_node_smoothness,
    std::vector<float> frame_smoothness) {
  using namespace std;

  map<string, float> node_smoothness;
  map<string, float> node_count;
  for (const auto &node: m_surfel_graph->nodes()) {
    node_smoothness[node->data()->id()] = -1;
    node_smoothness[node->data()->id()] = 0;
  }

  for (const auto &edge: m_surfel_graph->edges()) {
    const auto &from_surfel = edge.from()->data();
    const auto &to_surfel = edge.to()->data();
    auto common_frames = get_common_frames(from_surfel, to_surfel);
    for (auto frame_idx: common_frames) {
      auto smoothness = compute_smoothness_in_frame(edge, frame_idx);
      frame_smoothness[frame_idx] += smoothness;
      auto from_smoothness = node_smoothness[from_surfel->id()];
      node_smoothness[from_surfel->id()] = fmaxf(from_smoothness, 0) + smoothness;
      node_count[from_surfel->id()] = node_count[from_surfel->id()] + 1;
      auto to_smoothness = node_smoothness[to_surfel->id()];
      node_smoothness[to_surfel->id()] = fmaxf(to_smoothness, 0) + smoothness;
      node_count[to_surfel->id()] = node_count[to_surfel->id()] + 1;
    }
  }

  auto total_smoothness = 0.0f;
  auto total_count = 0.0f;
  for (auto &n: m_surfel_graph->nodes()) {
    total_smoothness += node_smoothness[n->data()->id()];
    total_count += node_count[n->data()->id()];
    auto mean_node_smoothness = node_smoothness[n->data()->id()] / node_count[n->data()->id()];
    store_mean_smoothness(n, mean_node_smoothness);
  }
  mean_node_smoothness = total_smoothness / total_count;
}

// Extract neighbour/frames per node and edges per frame
void
AbstractOptimiser::extract_graph_statistics() {

}

void
AbstractOptimiser::set_data(const SurfelGraphPtr &surfel_graph) {
  m_surfel_graph = surfel_graph;
  m_num_frames = get_num_frames(surfel_graph);
  m_frame_smoothness.resize(m_num_frames);
  m_smoothness_is_current = false;
  m_state = INITIALISED;

  extract_graph_statistics();
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

/*
 * Retrieve the set of frame indices for common frames of two nodes
 */
std::vector<unsigned int>
AbstractOptimiser::get_common_frames(
    const std::shared_ptr<Surfel> &s1,
    const std::shared_ptr<Surfel> &s2
) {
  using namespace std;

  set<unsigned int> s1_frames, s2_frames;
  s1_frames.insert(begin(s1->frames()), end(s1->frames()));
  s2_frames.insert(begin(s2->frames()), end(s2->frames()));
  vector<unsigned int> shared_frames(min(s1_frames.size(), s2_frames.size()));
  auto it = set_intersection(s1_frames.begin(), s1_frames.end(),
                             s2_frames.begin(), s2_frames.end(),
                             shared_frames.begin());
  shared_frames.resize(it - shared_frames.begin());
  return shared_frames;
}
