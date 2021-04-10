//
// Created by Dave Durbin (Old) on 2/4/21.
//

#include "surfel_graph_geometry_extractor.h"

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
        const SurfelGraphPtr graph,
        std::vector<float>& positions,
        std::vector<float>& tangents,
        std::vector<float>& normals) const {

    positions.clear();
    tangents.clear();
    normals.clear();

    for( const auto& node : graph->nodes()) {
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
}
