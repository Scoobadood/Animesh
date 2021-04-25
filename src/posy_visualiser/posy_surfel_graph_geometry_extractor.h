//
// Created by Dave Durbin (Old) on 2/4/21.
//

#pragma once

#include <Surfel/SurfelGraph.h>

class posy_surfel_graph_geometry_extractor {
public:
    posy_surfel_graph_geometry_extractor(float rho, float splatSize);

    void extract_geometry(const SurfelGraphPtr &graphPtr,
                          std::vector<float> &positions,
                          std::vector<float> &quads,
                          std::vector<float> &normals,
                          std::vector<float> &uvs
    ) const;

    inline void set_frame(int frame) {
        if (m_frame != frame) {
            m_frame = frame;
        }
    }

    inline int get_frame() const {
        return m_frame;
    }

    inline void setSplatSize(float splatSize) {
        if (m_splatSize != splatSize) {
            m_splatSize = splatSize;
        }
    }

private:
    int m_frame;
    float m_rho;
    float m_splatSize;
};
