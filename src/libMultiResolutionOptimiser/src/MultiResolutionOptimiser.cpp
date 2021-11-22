//
// Created by Dave Durbin (Old) on 25/10/21.
//

#include "MultiResolutionOptimiser.h"

MultiResolutionOptimiser::MultiResolutionOptimiser(const Properties &properties, std::default_random_engine &rng) //
    : Optimiser{properties, rng} //
    , m_current_level{0} //
    , m_state{UNINITIALISED} //
{
  m_num_levels = properties.getIntProperty("num-levels");
  m_rosy_optimiser = std::make_unique<RoSyOptimiser>(m_properties, rng);
  m_posy_optimiser = std::make_unique<PoSyOptimiser>(m_properties, rng);
}

bool
MultiResolutionOptimiser::optimise_do_one_step() {
  assert(m_state != UNINITIALISED);

  if( m_state == INITIALISED) {
    m_state = OPTIMISING_ROSY;
  }

  bool completed_optimisation;
  if (m_state == OPTIMISING_ROSY) {
    completed_optimisation = m_rosy_optimiser->optimise_do_one_step();
    if (!completed_optimisation) {
      return false;
    }
    m_state = OPTIMISING_POSY;
    completed_optimisation = false;
  }

  if (m_state == OPTIMISING_POSY) {
    completed_optimisation = m_posy_optimiser->optimise_do_one_step();
    if (!completed_optimisation) {
      return false;
    }
    if (m_current_level != 0) {
      m_multi_res_graph->propagate(m_current_level);
      --m_current_level;
      m_surfel_graph = (*m_multi_res_graph)[m_current_level];
      m_rosy_optimiser->set_data(m_surfel_graph);
      m_posy_optimiser->set_data(m_surfel_graph);
      m_state = OPTIMISING_ROSY;
      return false;
    }
    m_state = ENDING_OPTIMISATION;
  }

  if (m_state == ENDING_OPTIMISATION) {
    return true;
  }
  return false;
}

void
MultiResolutionOptimiser::set_data(const SurfelGraphPtr &surfel_graph) {
  m_multi_res_graph = std::make_unique<MultiResolutionSurfelGraph>(surfel_graph, m_random_engine);
  m_multi_res_graph->generate_levels(m_num_levels);
  m_current_level = m_num_levels - 1;
  m_surfel_graph = (*m_multi_res_graph)[m_current_level];
  m_rosy_optimiser->set_data(m_surfel_graph);
  m_posy_optimiser->set_data(m_surfel_graph);
  m_state = INITIALISED;
}
