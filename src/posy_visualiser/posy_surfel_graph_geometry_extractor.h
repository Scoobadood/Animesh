//
// Created by Dave Durbin (Old) on 2/4/21.
//

#pragma once

#include <Surfel/SurfelGraph.h>

class posy_surfel_graph_geometry_extractor {
public:
    explicit posy_surfel_graph_geometry_extractor(float rho);

    void extract_geometry(const SurfelGraphPtr &graphPtr,
                          std::vector<float> &positions,
                          std::vector<float> &quads,
                          std::vector<float> &triangle_fans,
                          std::vector<float> &triangle_uvs,
                          std::vector<unsigned int> &fan_sizes,
                          std::vector<float> &normals,
                          std::vector<float> &splat_sizes,
                          std::vector<float> &uvs
    ) const;

    inline void set_frame(int frame) {
        if (m_frame != frame) {
            m_frame = frame;
        }
    }

    // aim for 0.5f to 1.5f
    inline void set_splat_scale_factor(float splat_scale_factor) {
        splat_scale_factor = std::fminf(1.5f, std::fmaxf(0.5f, splat_scale_factor));
        if (m_splat_scale_factor != splat_scale_factor) {
            m_splat_scale_factor = splat_scale_factor;
        }
    }

    inline int get_frame() const {
        return m_frame;
    }

private:
    int m_frame;
    float m_rho;
    float m_splat_scale_factor;
};
