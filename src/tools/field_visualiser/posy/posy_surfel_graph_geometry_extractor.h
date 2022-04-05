//
// Created by Dave Durbin (Old) on 2/4/21.
//

#pragma once

#include <Surfel/SurfelGraph.h>
#include "geometry_extractor.h"

class posy_surfel_graph_geometry_extractor : public geometry_extractor {
public:
    explicit posy_surfel_graph_geometry_extractor(float rho);

    void extract_geometry(const SurfelGraphPtr &graphPtr,
                          std::vector<float> &positions,
                          std::vector<float> &lattice_positions,
                          std::vector<float> &quads,
                          std::vector<float> &triangle_fans,
                          std::vector<float> &triangle_uvs,
                          std::vector<unsigned int> &fan_sizes,
                          std::vector<float> &normals,
                          std::vector<float> &splat_sizes,
                          std::vector<float> &uvs
    ) const;

    // aim for 0.5f to 1.5f
    inline void set_splat_scale_factor(float splat_scale_factor) {
        splat_scale_factor = std::fminf(1.5f, std::fmaxf(0.5f, splat_scale_factor));
        if (m_splat_scale_factor != splat_scale_factor) {
            m_splat_scale_factor = splat_scale_factor;
        }
    }

private:
    float m_rho;
    float m_splat_scale_factor;
};
