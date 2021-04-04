//
// Created by Dave Durbin (Old) on 2/4/21.
//

#include "surfel_graph_geometry_extractor.h"
#include <Geom/Geom.h>

surfel_graph_geometry_extractor::surfel_graph_geometry_extractor() {
    m_frame = 0;
}

void extract_xyz_triples_for_frame(const SurfelGraph& graph,
                                   unsigned int frame,
                                   std::vector<float>& positions,
                                   std::vector<float>& tangents,
                                   std::vector<float>& normals) {
    for( const auto& node : graph.nodes()) {
        const auto& surfel = node->data();
        if(!surfel->is_in_frame(frame)) {
            continue;
        }

        Eigen::Vector3f position, normal, tangent;
        surfel->get_position_tangent_normal_for_frame(frame, position, tangent, normal);

        positions.push_back(position.x());
        positions.push_back(position.y());
        positions.push_back(position.z());

        tangents.push_back(tangent.x());
        tangents.push_back(tangent.y());
        tangents.push_back(tangent.z());

        normals.push_back(normal.x());
        normals.push_back(normal.y());
        normals.push_back(normal.z());
    }
}

void centre_at_origin(std::vector<float>& xyz) {
    auto centroidX = 0.0f;
    auto centroidY = 0.0f;
    auto centroidZ = 0.0f;
    compute_centroid(xyz, centroidX, centroidY, centroidZ);
    for( unsigned int i=0; i<xyz.size() / 3; i += 3) {
        xyz.at(i + 0) -= centroidX;
        xyz.at(i + 1) -= centroidY;
        xyz.at(i + 2) -= centroidZ;
    }
}

float scale_to_region(std::vector<float>& xyz) {
    auto minX = MAXFLOAT;
    auto minY = MAXFLOAT;
    auto minZ = MAXFLOAT;
    auto maxX = -MAXFLOAT;
    auto maxY = -MAXFLOAT;
    auto maxZ = -MAXFLOAT;
    compute_bounds(xyz, minX, maxX, minY, maxY, minZ, maxZ);

    auto rangeX = maxX - minX;
    auto rangeY = maxY - minY;
    auto rangeZ = maxZ - minZ;
    auto range = fmaxf(fmaxf( rangeX, rangeY), rangeZ);
    auto scale = 1.0f / range;
    for( unsigned int i=0; i<xyz.size(); ++i) {
        xyz.at(i) *= scale;
    }
    return scale;
}

float compute_tan_scale(const SurfelGraph& graph, float scale) {
    // Pick a surfel and compute mean neighbour distance for this frame
    const auto node = graph.nodes().front();
    const auto surfel = node->data();
    const auto neighbours = graph.neighbours(node);
    int count = 0;
    float distance = 0.0f;
    for( const auto& fd : surfel->frame_data) {
        unsigned int frame = fd.pixel_in_frame.frame;
        const auto& pos = fd.position * scale;

        for( const auto& node : neighbours) {
            const auto& otherSurfel = node->data();
            if( otherSurfel->is_in_frame(frame)) {
                Eigen::Vector3f position, tangent, normal;
                otherSurfel->get_position_tangent_normal_for_frame(frame, position, tangent, normal);
                distance += (distance_from_point_to_point(position * scale, pos));
                count++;
            }
        }
    }
    const auto mean_neighbour_distance = (count > 0)
        ? distance/ count
        : 1.0f;

    // Proposed scale should be 2/5 of mean neighbour distance so that
    // in general, adjacent tangents don't touch.
    return mean_neighbour_distance * 0.4f;
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
void surfel_graph_geometry_extractor::extract_geometry(
        const SurfelGraph& graph,
        std::vector<float>& positions,
        std::vector<float>& tangents,
        std::vector<float>& normals,
        float& scale_factor
        ) const {

    positions.clear();
    tangents.clear();
    normals.clear();

    extract_xyz_triples_for_frame(graph, m_frame, positions, tangents, normals);
//    centre_at_origin(positions);
//    const auto scale = scale_to_region(positions);
const auto scale = 1.0f;
    scale_factor = compute_tan_scale(graph, scale);
}
