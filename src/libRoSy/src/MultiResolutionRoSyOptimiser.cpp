//
// Created by Dave Durbin (Old) on 7/10/21.
//

#include "MultiResolutionRoSyOptimiser.h"
#include <Surfel/Surfel_IO.h>

MultiResolutionRoSyOptimiser::MultiResolutionRoSyOptimiser(const Properties &properties,
                                                           std::default_random_engine &rng) //
    : RoSyOptimiser{properties, rng} //
    , m_multi_res_graph{nullptr} //
    , m_current_level{0} //
{
  m_num_levels = properties.getIntProperty("num-levels");
  m_save_interim_graphs = properties.hasProperty("rosy-debug-save-interim-graphs")
      && properties.getBooleanProperty("rosy-debug-save-interim-graphs");
}

void
MultiResolutionRoSyOptimiser::loaded_graph() {
  m_multi_res_graph = new MultiResolutionSurfelGraph{m_surfel_graph, m_random_engine};
  m_multi_res_graph->generate_levels(m_num_levels);
  if (m_save_interim_graphs) {
    for (int l = 0; l < m_num_levels; ++l) {
      std::string output_file_name = "start_level_" + std::to_string(l) + ".bin";
      save_surfel_graph_to_file(output_file_name, (*m_multi_res_graph)[l], true, true);
    }
  }

  m_current_level = m_num_levels - 1;
  m_surfel_graph = (*m_multi_res_graph)[m_current_level];
}

/* Call back when termination criteria are met */
void MultiResolutionRoSyOptimiser::smoothing_completed(float smoothness, OptimisationResult result) {
  if (m_current_level == 0) {
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
  if (m_save_interim_graphs) {
      std::string output_file_name = "end_level_" + std::to_string(m_current_level) + ".bin";
      save_surfel_graph_to_file(output_file_name, (*m_multi_res_graph)[m_current_level], true, true);
  }

  m_multi_res_graph->propagate(m_current_level);
  --m_current_level;
  m_surfel_graph = (*m_multi_res_graph)[m_current_level];
  m_result = NOT_COMPLETE;
  m_state = INITIALISED;
}
