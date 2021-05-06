//
// Created by Dave Durbin (Old) on 2/4/21.
//

#include "posy_surfel_graph_geometry_extractor.h"
#include <Geom/Geom.h>

posy_surfel_graph_geometry_extractor::posy_surfel_graph_geometry_extractor(float rho)
        : m_frame{0}, m_rho{rho} {
}

void
compute_distance_to_neighbours(const SurfelGraphNodePtr &node, const SurfelGraphPtr &graphPtr,
                                          unsigned int frame, float& mean_nd, float& min_nd) {
    unsigned int num_neighbours = 0;
    Eigen::Vector3f vertex, normal, tangent, t2, reference_lattice_vertex;
    node->data()->get_vertex_tangent_normal_for_frame(frame, vertex, tangent, normal);

    float min_distance = MAXFLOAT;
    float total_distance = 0.0f;
    for (const auto &n : graphPtr->neighbours(node)) {
        if (!n->data()->is_in_frame(frame)) {
            continue;
        }

        Eigen::Vector3f other_vertex, other_normal, other_tangent;
        n->data()->get_vertex_tangent_normal_for_frame(frame, other_vertex, other_tangent, other_normal);
        const auto dist = (vertex - other_vertex).norm();
        total_distance += dist;
        if( dist < min_distance) {
            min_distance = dist;
        }
        ++num_neighbours;
    }

    mean_nd = num_neighbours == 0
           ? 1.0f
           : total_distance / (float) num_neighbours;
    min_nd = num_neighbours == 0
              ? 1.0f
              : min_distance;
}

void extract_quads_for_frame(const SurfelGraphPtr &graphPtr,
                             unsigned int frame,
                             std::vector<float> &positions,
                             std::vector<float> &quads,
                             std::vector<float> &normals,
                             std::vector<float> &splat_sizes,
                             std::vector<float> &uvs,
                             float rho,
                             float splat_scale_factor) {

    for (const auto &node : graphPtr->nodes()) {
        const auto &surfel = node->data();
        if (!surfel->is_in_frame(frame)) {
            continue;
        }

        Eigen::Vector3f vertex, normal, tangent, t2, reference_lattice_vertex;
        surfel->get_all_data_for_surfel_in_frame(frame, vertex, tangent, t2, normal, reference_lattice_vertex);

        // Stash the vertex vertex
        positions.push_back(vertex.x());
        positions.push_back(vertex.y());
        positions.push_back(vertex.z());

        // ... and normal ...
        normals.push_back(normal.x());
        normals.push_back(normal.y());
        normals.push_back(normal.z());

        // Scale quad based on proximity to neighbours
        float mean_nd, min_nd;
        compute_distance_to_neighbours(node, graphPtr, frame, mean_nd, min_nd);
        const auto splat_size = min_nd * splat_scale_factor;

        // Size of actual quads, centred on the vertex
        for (auto uv : std::vector<std::tuple<float, float>>{
                {-0.5f * splat_size, -0.5f * splat_size},
                {-0.5f * splat_size, 0.5f * splat_size},
                {0.5f * splat_size,  0.5f * splat_size},
                {0.5f * splat_size,  -0.5f * splat_size},
        }) {
            const auto p = vertex
                           + (std::get<0>(uv) * tangent)
                           + (std::get<1>(uv) * t2);
            quads.push_back(p.x());
            quads.push_back(p.y());
            quads.push_back(p.z());
        }
        splat_sizes.push_back(splat_size);

        // UVs are texture coords
        // ref_lat_offset indicates the centre of the cross.
        // also ref_lat_off is in range [-rho/2 , rho/2)
        // We need to divide by rho and add 0.5 to make them in the range
        // 0->1
        auto texture_offset = surfel->reference_lattice_offset();
        auto norm_texture_offset = texture_offset/ rho; // -0.5 to 0.5
        float u,v;
        if (norm_texture_offset[0] < 0.0) {
            u = - norm_texture_offset[0];
        } else {
            u = 1.0f - norm_texture_offset[0];
        }
        if (norm_texture_offset[1] < 0.0) {
            v = - norm_texture_offset[1];
        } else {
            v = 1.0f - norm_texture_offset[1];
        }
        uvs.push_back(u);
        uvs.push_back(v);
    }
}

/**
 * @brief Generate the vertex data needed to render the current frame of this graph.
 * All float vectors are populated in x,y,z coordinate triplets.
 * @param graph The Surfel Graph
 * @param positions a vector into which the Surfel positions will be pushed.
 * @param tangents A Vector of XYZ coordinates for the unit indicative tangent.
 * @param normals A Vector of XYZ coordinates for the unit normal.
 * @param scaleFactor A proposed scaling for the normals and tangents.
 */
void posy_surfel_graph_geometry_extractor::extract_geometry(
        const SurfelGraphPtr &graphPtr,
        std::vector<float> &positions,
        std::vector<float> &quads,
        std::vector<float> &normals,
        std::vector<float> &splat_sizes,
        std::vector<float> &uvs
) const {

    positions.clear();
    normals.clear();
    quads.clear();
    splat_sizes.clear();
    uvs.clear();

    extract_quads_for_frame(graphPtr, m_frame, positions, quads, normals, splat_sizes, uvs, m_rho, m_splat_scale_factor);
}
