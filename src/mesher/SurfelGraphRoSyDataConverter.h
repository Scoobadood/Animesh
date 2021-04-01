//
// Created by Dave Durbin (Old) on 31/3/21.
//

#pragma once

#include <vector>
#include <Surfel/SurfelGraph.h>

class SurfelGraphRoSyDataConverter {
private:
    int frame;
public:
    void extractRenderGeometry(const SurfelGraphPtr& graph, std::vector<float>& vertices, std::vector<float>& colours);
};
