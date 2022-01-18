//
// Created by Dave Durbin (Old) on 2/4/21.
//

#pragma once

#include <Surfel/SurfelGraph.h>
#include "geometry_extractor.h"

class rosy_surfel_graph_geometry_extractor
    : public geometry_extractor {
  float m_spur_length;
public:
  rosy_surfel_graph_geometry_extractor() : m_spur_length{1.0f}{}

  void set_spur_length(float spur_length) {
    m_spur_length = spur_length;
  }
  void extract_geometry(const SurfelGraphPtr &graphPtr,
                        std::vector<float> &positions,
                        std::vector<float> &tangents,
                        std::vector<float> &normals,
                        std::vector<float> &colours,
                        std::vector<float> &path,
                        float &scale_factor) const;
};
