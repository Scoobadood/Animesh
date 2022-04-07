//
// Created by Dave Durbin on 8/4/2022.
//

#include "FieldOptimiser.h"
#include <spdlog/spdlog.h>

FieldOptimiser::FieldOptimiser() //
    : m_graph{nullptr} //
    , m_state{UNINITIALISED} //
{}

void
FieldOptimiser::optimise_begin() {
  spdlog::trace("optimise_begin()");
  assert(m_state == INITIALISED);

  m_current_level = m_graph->num_levels() - 1;
  m_num_iterations = 0;
  m_state = OPTIMISING_ROSY;
}

void
optimise_rosy() {
}

bool
FieldOptimiser::optimise_once() {
  bool optimisation_complete = false;

  switch (m_state) {
    case UNINITIALISED:throw std::logic_error("Can't optimise when no graph is set");
    case INITIALISED:optimise_begin();
      break;
    case OPTIMISING_ROSY:
      optimise_rosy();
      break;

  }
  return optimisation_complete;
}

void
FieldOptimiser::set_graph(std::shared_ptr<MultiResolutionSurfelGraph> graph) {
  m_graph = graph;
  m_state = INITIALISED;
}

