//
// Created by Dave Durbin (Old) on 7/10/21.
//

#include "MultiResolutionPoSyOptimiser.h"

MultiResolutionPoSyOptimiser::MultiResolutionPoSyOptimiser(const Properties &properties, std::default_random_engine& rng) //
    : PoSyOptimiser{properties, rng} //
    , m_multi_res_graph{nullptr} //
    , m_current_level{0} //
{
  m_num_levels = properties.getIntProperty("num-levels");
}

void
MultiResolutionPoSyOptimiser::loaded_graph() {
  m_multi_res_graph = new MultiResolutionSurfelGraph{m_surfel_graph, m_random_engine};
  m_multi_res_graph->generate_levels(m_num_levels);
  m_current_level = m_num_levels - 1;
  m_surfel_graph = (*m_multi_res_graph)[m_current_level];
}

/* Call back when termination criteria are met */
void MultiResolutionPoSyOptimiser::smoothing_completed(float smoothness, OptimisationResult result) {
  if( m_current_level == 0 ) {
    spdlog::info("Multi-resolution terminating because {}. Smoothness : {:4.3f}",
                 m_result == NOT_COMPLETE
                 ? "not complete"
                 : m_result == CONVERGED
                   ? "converged"
                   : "cancelled", smoothness
    );
    return;
  }

  spdlog::info("Multi-resolution completed level {} because {}. Smoothness : {:4.3f}",
               m_current_level,
               m_result == NOT_COMPLETE
               ? "not complete"
               : m_result == CONVERGED
                 ? "converged"
                 : "cancelled", smoothness
  );

  m_multi_res_graph->propagate(m_current_level, false, true);
  --m_current_level;
  m_surfel_graph = (*m_multi_res_graph)[m_current_level];
  m_result = NOT_COMPLETE;
  m_state = INITIALISED;
}
