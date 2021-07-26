//
// Created by Dave Durbin (Old) on 12/7/21.
//

#include "Quad.h"

#include <PoSy/PoSy.h>
#include <RoSy/RoSy.h>
#include <Surfel/SurfelGraph.h>
#include <map>
#include <Eigen/Geometry>

std::shared_ptr<animesh::Graph<Eigen::Vector3f, EdgeType>>
build_edge_graph(
        int frame_index,
        const SurfelGraphPtr &graph,
        float rho
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
            Vector3f vertex, tangent, normal;
            source_node->data()->get_vertex_tangent_normal_for_frame(frame_index, vertex, tangent, normal);

            for (const auto &neighbour_node : graph->neighbours(source_node)) {
                const auto &edge = graph->edge(source_node, neighbour_node);
                const auto &t_ij = edge->t_ij(frame_index);
                const auto &t_ji = edge->t_ji(frame_index);


                Vector3f nbr_vertex, nbr_tangent, nbr_normal;
                neighbour_node->data()->get_vertex_tangent_normal_for_frame(frame_index, nbr_vertex, nbr_tangent, nbr_normal);

                // Recompute all the things
                auto best_rot = best_rosy_vector_pair(tangent, normal, nbr_tangent, nbr_normal);
                auto best_shift = best_posy_offset(vertex, tangent, normal, source_node->data()->reference_lattice_offset(),
                                                   nbr_vertex, nbr_tangent, nbr_normal, neighbour_node->data()->reference_lattice_offset(), rho);

                const auto absDiff = (best_shift.first - best_shift.second).cwiseAbs();
                if (absDiff.maxCoeff() > 1 || (absDiff == Vector2i(1, 1) ))
                    continue; /* Ignore longer-distance links and diagonal lines for quads */

                if (absDiff.sum() != 0) {
                    spdlog::info( "Adding red edge for t_ij:({},{}) , t_ji:({},{})",
//                                  t_ij.x(), t_ij.y(), t_ji.x(), t_ji.y());
                                  best_shift.first.x(), best_shift.first.y(), best_shift.second.x(), best_shift.second.y());
                    out_graph->add_edge(
                            vertex_to_node.at(source_node->data()->id()),
                            vertex_to_node.at(neighbour_node->data()->id()),
                            EDGE_TYPE_RED
                    );
                }
                // If t_ij == -t_ij then these are blue edges
                else {
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

std::shared_ptr<animesh::Graph<Eigen::Vector3f, EdgeType>>
collapse(int frame_index,
        SurfelGraphPtr &graph,
        float rho
) {
    for( const auto& edge : graph->edges()) {
        // Collapse the blue edges
        // For each edge, we will find the two vertices. We must align them. Each frame is currently a separate graph.
        // Collapsing vertices meansadding new edges (inheriting their
    }
}