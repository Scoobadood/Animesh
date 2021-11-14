//
// Created by Dave Durbin (Old) on 8/11/21.
//

#include "NodeOptimiser.h"
#include "SurfelSelectionAlgorithm.h"

#include <random>
#include <sstream>
#include <utility>
#include <algorithm>    // random_shuffle
#include <sys/stat.h>
#include <spdlog/spdlog.h>
#include <Properties/Properties.h>       // termination criteria set up

NodeOptimiser::NodeOptimiser(Properties properties, std::default_random_engine& rng)
    : AbstractOptimiser{properties, rng}//
    , m_randomise_neighour_order{false} //
    , m_node_selection_function{nullptr} //
    , m_ssa_percentage{0} //
{
}

void
NodeOptimiser::setup_ssa() {
  m_node_selection_function = nullptr;
  auto ssa = ::with_name(m_properties.getProperty(get_ssa_property_name()));
  switch (ssa) {
  case SSA_SELECT_ALL_IN_RANDOM_ORDER:
    m_node_selection_function = std::bind(&NodeOptimiser::ssa_select_all_in_random_order, this);
    break;

  case SSA_SELECT_WORST_100:m_node_selection_function = std::bind(&NodeOptimiser::ssa_select_worst_100, this);
    break;

  case SSA_SELECT_WORST_PERCENTAGE:
    auto ssa_percentage = m_properties.getIntProperty(get_ssa_percentage_property_name());
    m_ssa_percentage = std::min<float>(std::max<float>(0.0f, ((float)ssa_percentage / 100.0f)), 1.0f);
    m_node_selection_function = std::bind(&NodeOptimiser::ssa_select_worst_percentage, this);
    break;
  }
}

void NodeOptimiser::optimise_do_pass() {
  auto nodes_to_optimise = select_nodes_to_optimise();
  for (const auto &node: nodes_to_optimise) {
    optimise_node(node);
  }
}

std::vector<SurfelGraphNodePtr>
NodeOptimiser::select_nodes_to_optimise() {
  assert(m_node_selection_function);
  return m_node_selection_function(*this);
}

/**
 * Select all surfels and randomize the order
 */
std::vector<SurfelGraphNodePtr>
NodeOptimiser::ssa_select_all_in_random_order() {
  using namespace std;

  vector<size_t> indices(m_surfel_graph->num_nodes());
  iota(begin(indices), end(indices), 0);
  shuffle(begin(indices), end(indices), m_random_engine);
  vector<SurfelGraphNodePtr> selected_nodes;
  selected_nodes.reserve(indices.size());
  const auto graph_nodes = m_surfel_graph->nodes();
  for (const auto i: indices) {
    selected_nodes.push_back(graph_nodes.at(i));
  }
  return selected_nodes;
}

/**
 * Surfel selection model 2: Select least smooth 100
 */
std::vector<SurfelGraphNodePtr>
NodeOptimiser::ssa_select_worst_100() const {
  using namespace std;

  vector<SurfelGraphNodePtr> selected_nodes;
  for (const auto &n: m_surfel_graph->nodes()) {
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
NodeOptimiser::ssa_select_worst_percentage() const {
  using namespace std;

  vector<SurfelGraphNodePtr> selected_nodes;
  for (const auto &n: m_surfel_graph->nodes()) {
    selected_nodes.push_back(n);
  }
  // Sort worst to best
  stable_sort(begin(selected_nodes),
              end(selected_nodes),
              [this](const SurfelGraphNodePtr &l, const SurfelGraphNodePtr &r) {
                return compare_worst_first(l, r);
              });

  // Required nodes
  auto required_nodes = (unsigned int) std::roundf( (float)m_surfel_graph->num_nodes() *  m_ssa_percentage);
  if (selected_nodes.size() > required_nodes) {
    selected_nodes.resize(required_nodes);
  }
  return selected_nodes;
}
