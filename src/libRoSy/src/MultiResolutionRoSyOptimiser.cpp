//
// Created by Dave Durbin (Old) on 7/10/21.
//

#include "../include/RoSy/MultiResolutionRoSyOptimiser.h"

MultiResolutionRoSyOptimiser::MultiResolutionRoSyOptimiser(const Properties &properties) //
    : Optimiser{properties} //
    , m_properties{properties} //
{
  m_num_levels = properties.getIntProperty("num-levels");
}

void
MultiResolutionRoSyOptimiser::loaded_graph() {
  m_multi_res_graph = new MultiResolutionSurfelGraph{m_surfel_graph};
  m_multi_res_graph->generate_levels(m_num_levels);
  m_surfel_graph = m_multi_res_graph->back();
}
