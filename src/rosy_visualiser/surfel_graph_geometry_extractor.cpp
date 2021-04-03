//
// Created by Dave Durbin (Old) on 2/4/21.
//

#include "surfel_graph_geometry_extractor.h"
#include <Geom/Geom.h>

surfel_graph_geometry_extractor::surfel_graph_geometry_extractor() {
    m_frame = 0;
}

/**
 * Generate the vertex, colour and index data needed to render the currentframe of this graph
 * @param graph The graph to extract data from
 * @param vertex_data Vertices, specified as 3xfloat X,Y,Z then 4xfloat RGBA
 * @param index_data Indices for use with VAO to draw lines
 */
void surfel_graph_geometry_extractor::extract_geometry(
        const SurfelGraph& graph,
        std::vector<float>& positions,
        std::vector<float>& tangents,
        std::vector<float>& normals) const {

    positions.clear();
    tangents.clear();
    normals.clear();

    for( const auto& node : graph.nodes()) {
        const auto& surfel = node->data();
        if(!surfel->is_in_frame(m_frame)) {
            continue;
        }

        Eigen::Vector3f position, normal, tangent;
        surfel->get_position_tangent_normal_for_frame(m_frame, position, tangent, normal);

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

    auto centroidX = 0.0f;
    auto centroidY = 0.0f;
    auto centroidZ = 0.0f;
    compute_centroid(positions, centroidX, centroidY, centroidZ);
    for( unsigned int i=0; i<positions.size() / 3; i += 3) {
        positions.at(i + 0) -= centroidX;
        positions.at(i + 1) -= centroidY;
        positions.at(i + 2) -= centroidZ;
    }

    auto minX = MAXFLOAT;
    auto minY = MAXFLOAT;
    auto minZ = MAXFLOAT;
    auto maxX = -MAXFLOAT;
    auto maxY = -MAXFLOAT;
    auto maxZ = -MAXFLOAT;
    compute_bounds(positions, minX, maxX, minY, maxY, minZ, maxZ);

    auto rangeX = maxX - minX;
    auto rangeY = maxY - minY;
    auto rangeZ = maxZ - minZ;
    auto range = fmaxf(fmaxf( rangeX, rangeY), rangeZ);
    auto scale = 1.0f / range;
    for( unsigned int i=0; i<positions.size(); ++i) {
        positions.at(i) *= scale;
    }

    // Scale tangents and normals
    // Pick a random surfel and compute mean neighbour distance for this frame
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
    float tanScale = count > 0
        ? ((distance * scale)/ count)
        : scale;
//    tanScale  = tanScale  * tanScale;
    for( unsigned int i=0; i<tangents.size(); i++ ) {
        tangents.at(i) *= tanScale;
        normals.at(i) *= tanScale;
    }
}
