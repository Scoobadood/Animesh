//
// Created by Dave Durbin (Old) on 2/4/21.
//

#pragma once

#include <Surfel/SurfelGraph.h>
#include "geometry_extractor.h"

class rosy_surfel_graph_geometry_extractor : public geometry_extractor {
public:
    void extract_geometry(const SurfelGraphPtr& graphPtr,
                          std::vector<float>& positions,
                          std::vector<float>& tangents,
                          std::vector<float>& normals,
                          std::vector<float>& colours,
                          float& scale_factor) const;
};
