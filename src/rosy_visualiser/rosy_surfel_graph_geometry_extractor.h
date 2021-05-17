//
// Created by Dave Durbin (Old) on 2/4/21.
//

#pragma once

#include <Surfel/SurfelGraph.h>

class rosy_surfel_graph_geometry_extractor {
public:
    rosy_surfel_graph_geometry_extractor( );
    void extract_geometry(const SurfelGraphPtr& graphPtr,
                          std::vector<float>& positions,
                          std::vector<float>& tangents,
                          std::vector<float>& normals,
                          float& scale_factor) const;

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
