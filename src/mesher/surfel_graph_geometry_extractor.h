//
// Created by Dave Durbin (Old) on 2/4/21.
//

#pragma once

#include <Surfel/SurfelGraph.h>

class surfel_graph_geometry_extractor {
public:
    surfel_graph_geometry_extractor( );
    void extract_geometry(SurfelGraphPtr graph,
                          std::vector<float>& positions,
                          std::vector<float>& tangents,
                          std::vector<float>& normals) const;
    inline void set_frame(int frame) {
        if(m_frame != frame) {
            m_frame = frame;
        }
    }
    inline int get_frame() const {
        return m_frame;
    }

private:
    int m_frame;
};