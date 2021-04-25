//
// Created by Dave Durbin (Old) on 2/4/21.
//

#include "posy_surfel_graph_geometry_extractor.h"
#include <Geom/Geom.h>

posy_surfel_graph_geometry_extractor::posy_surfel_graph_geometry_extractor(float rho, float splatSize)
: m_frame{0}, m_rho{rho}, m_splatSize{splatSize} {
}

void extract_quads_for_frame(const SurfelGraphPtr &graphPtr,
                             unsigned int frame,
                             std::vector<float> &positions,
                             std::vector<float> &quads,
                             std::vector<float> &normals,
                             std::vector<float> &uvs,
                             float rho,
                             float splatSize) {
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

        // Generate a quad with one vertex at the reference lattice vertex
        spdlog::info("Vertex : {:2f}, {:2f},{:2f}", vertex[0], vertex[1], vertex[2]);
        spdlog::info("  norm : {:2f}, {:2f},{:2f}", normal[0], normal[1], normal[2]);
        spdlog::info("   tan : {:2f}, {:2f},{:2f}", tangent[0], tangent[1], tangent[2]);
        spdlog::info("reflat : {:2f}, {:2f},{:2f}", reference_lattice_vertex[0], reference_lattice_vertex[1],
                     reference_lattice_vertex[2]);

        float scale = 1.0f / rho;

        // Size of actual quads, ceentred on the vertex
        for (auto uv : std::vector<std::tuple<float, float>>{
                {-0.5f * splatSize, -0.5f * splatSize},
                {-0.5f * splatSize, 0.5f * splatSize},
                {0.5f * splatSize,  0.5f * splatSize},
                {0.5f * splatSize,  -0.5f * splatSize},
        }) {
            const auto p = vertex
                           + (std::get<0>(uv) * tangent)
                           + (std::get<1>(uv) * t2);
            quads.push_back(p.x());
            quads.push_back(p.y());
            quads.push_back(p.z());
            spdlog::info("      Q : {:2f}, {:2f},{:2f}", p[0], p[1], p[2]);
        }


        // UVs are texture coords
        // ref_lat_offset indicates the centre of the cross.
        // also ref_lat_off is in range [-rho/2 , rho/2)
        // We need to divide by rho and add 0.5 to make them in the range
        // 0->1
        // The splat texture should have the vertex at 0,0
        auto texture_offset = (surfel->reference_lattice_offset() / rho);
        uvs.push_back( texture_offset[0]);
        uvs.push_back( texture_offset[1]);
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
        std::vector<float> &uvs
) const {

    positions.clear();
    normals.clear();
    quads.clear();
    uvs.clear();

    extract_quads_for_frame(graphPtr, m_frame, positions, quads, normals, uvs, m_rho, m_splatSize);
}
