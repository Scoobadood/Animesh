//
// Created by Dave Durbin (Old) on 2/4/21.
//

#include "posy_surfel_graph_geometry_extractor.h"
#include <Geom/Geom.h>

posy_surfel_graph_geometry_extractor::posy_surfel_graph_geometry_extractor() {
    m_frame = 0;
}

void extract_xyz_triples_for_frame(const SurfelGraphPtr& graphPtr,
                                   unsigned int frame,
                                   std::vector<float> &positions,
                                   std::vector<float> &normals,
                                   std::vector<float> &uvs) {
    for (const auto &node : graphPtr->nodes()) {
        const auto &surfel = node->data();
        if (!surfel->is_in_frame(frame)) {
            continue;
        }

        Eigen::Vector3f position, normal, tangent;
        surfel->get_position_tangent_normal_for_frame(frame, position, tangent, normal);
        auto uv = surfel->closest_mesh_vertex_offset;

        positions.push_back(position.x());
        positions.push_back(position.y());
        positions.push_back(position.z());

        normals.push_back(normal.x());
        normals.push_back(normal.y());
        normals.push_back(normal.z());

        uvs.push_back(uv.x());
        uvs.push_back(uv.y());
    }
}

void centre_at_origin(std::vector<float> &xyz) {
    auto centroidX = 0.0f;
    auto centroidY = 0.0f;
    auto centroidZ = 0.0f;
    compute_centroid(xyz, centroidX, centroidY, centroidZ);
    for (unsigned int i = 0; i < xyz.size(); i += 3) {
        xyz.at(i + 0) -= centroidX;
        xyz.at(i + 1) -= centroidY;
        xyz.at(i + 2) -= centroidZ;
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
        const SurfelGraphPtr& graphPtr,
        std::vector<float> &positions,
        std::vector<float> &normals,
        std::vector<float> &uvs
) const {

    positions.clear();
    normals.clear();
    uvs.clear();

    extract_xyz_triples_for_frame(graphPtr, m_frame, positions, normals, uvs);
    centre_at_origin(positions);
}
