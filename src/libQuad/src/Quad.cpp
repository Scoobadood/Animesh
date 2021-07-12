//
// Created by Dave Durbin (Old) on 12/7/21.
//

#include "Quad.h"

#include <Surfel/SurfelGraph.h>
#include <map>
#include <Eigen/Geometry>

std::shared_ptr<animesh::Graph<Eigen::Vector3f, EdgeType>>
build_edge_graph(
        int frame_index,
        const SurfelGraphPtr &graph
) {
    using namespace Eigen;
    using namespace std;

    shared_ptr<animesh::Graph<Vector3f, EdgeType>> out_graph = make_shared<animesh::Graph<Vector3f, EdgeType>>(false);

    map<string, shared_ptr<animesh::Graph<Vector3f, EdgeType>::GraphNode>> vertex_to_node;
    for (const auto &source_node : graph->nodes()) {
        if (source_node->data()->is_in_frame(frame_index)) {

            Vector3f src_vertex, src_tangent, src_normal;
            Vector2f offset = source_node->data()->reference_lattice_offset();
            source_node->data()->get_vertex_tangent_normal_for_frame(
                    frame_index, src_vertex, src_tangent, src_normal);
            Vector3f src_closest_mesh_vertex = src_vertex +
                                               (src_tangent * offset.x()) +
                                               ((src_normal.cross(src_tangent)) * offset.y());

            auto node = out_graph->add_node(src_closest_mesh_vertex);
            vertex_to_node.insert({source_node->data()->id(), node});
        }
    }

    // Do edges
    for (const auto &source_node : graph->nodes()) {
        if (source_node->data()->is_in_frame(frame_index)) {
            for (const auto &neighbour_node : graph->neighbours(source_node)) {
                const auto &edge = graph->edge(source_node, neighbour_node);
                const auto &t_ij = edge->t_ij(frame_index);
                const auto &t_ji = edge->t_ji(frame_index);
                const auto &t = t_ij + t_ji;

                if (t.squaredNorm() == 1) {
                    out_graph->add_edge(
                            vertex_to_node.at(source_node->data()->id()),
                            vertex_to_node.at(neighbour_node->data()->id()),
                            EDGE_TYPE_RED
                    );
                } else if (t.squaredNorm() == 0) {
                    out_graph->add_edge(
                            vertex_to_node.at(source_node->data()->id()),
                            vertex_to_node.at(neighbour_node->data()->id()),
                            EDGE_TYPE_BLU
                    );
                }
            }
        }
    }
    return out_graph;
}