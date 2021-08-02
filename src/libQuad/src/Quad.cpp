//
// Created by Dave Durbin (Old) on 12/7/21.
//

#include "Quad.h"

#include <PoSy/PoSy.h>
#include <RoSy/RoSy.h>
#include <Surfel/SurfelGraph.h>
#include <map>
#include <Eigen/Geometry>

QuadGraphPtr
build_edge_graph(
    int frame_index,
    const SurfelGraphPtr &graph,
    float rho
) {
  using namespace Eigen;
  using namespace std;

  // Make the output graph
  QuadGraphPtr out_graph = make_shared<animesh::Graph<Vector3f, EdgeType>>(false);

  // Output all lattice vertices and also map hem from surfel ID
  map<string, QuadGraphNodePtr> vertex_to_node;
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

  // Collect all edges and weed out duplicates.
  std::set<std::pair<std::string, std::string>> counted_edges;
  std::vector<SurfelGraph::Edge> edges;
  for (const auto &edge : graph->edges()) {
    if (!edge.from()->data()->is_in_frame(frame_index)) {
      continue;
    }
    if (!edge.to()->data()->is_in_frame(frame_index)) {
      continue;
    }
    if (counted_edges.count({edge.from()->data()->id(), edge.to()->data()->id()}) > 0) {
      continue;
    }
    if (counted_edges.count({edge.to()->data()->id(), edge.from()->data()->id()}) > 0) {
      continue;
    }
    counted_edges.insert({edge.from()->data()->id(), edge.to()->data()->id()});
    edges.push_back(edge);
  }

  // So edges contains only edges in this frame and each only once
  for (const auto &edge : edges) {
    const auto & from_surfel = edge.from()->data();
    const auto & to_surfel = edge.to()->data();

    Vector3f vertex, tangent, normal;
    from_surfel->get_vertex_tangent_normal_for_frame(frame_index, vertex, tangent, normal);

    Vector3f nbr_vertex, nbr_tangent, nbr_normal;
    to_surfel->get_vertex_tangent_normal_for_frame(frame_index, nbr_vertex, nbr_tangent, nbr_normal);

    const auto &t_ij = edge.data()->t_ij(frame_index);
    const auto &t_ji = edge.data()->t_ji(frame_index);

    // Recompute all the things
    auto best_rot = best_rosy_vector_pair(tangent, normal, nbr_tangent, nbr_normal);
    auto best_shift = best_posy_offset(vertex, tangent, normal,
                                       from_surfel->reference_lattice_offset(),
                                       nbr_vertex, nbr_tangent, nbr_normal,
                                       to_surfel->reference_lattice_offset(), rho);

    const auto absDiff = (best_shift.first - best_shift.second).cwiseAbs();
    if (absDiff.maxCoeff() > 1 || (absDiff == Vector2i(1, 1)))
      continue; /* Ignore longer-distance links and diagonal lines for quads */

    if (absDiff.sum() != 0) {
      spdlog::info("Adding red edge for t_ij:({},{}) , t_ji:({},{})",
//                                  t_ij.x(), t_ij.y(), t_ji.x(), t_ji.y());
                   best_shift.first.x(), best_shift.first.y(), best_shift.second.x(),
                   best_shift.second.y());
      out_graph->add_edge(
          vertex_to_node.at(edge.from()->data()->id()),
          vertex_to_node.at(edge.to()->data()->id()),
          EDGE_TYPE_RED
      );
    }
      // If t_ij == -t_ij then these are blue edges
    else {
      out_graph->add_edge(
          vertex_to_node.at(from_surfel->id()),
          vertex_to_node.at(to_surfel->id()),
          EDGE_TYPE_BLU
      );
    }
  }
  return out_graph;
}

void
collapse(int frame_index,
         QuadGraphPtr graph,
         float rho
) {
  using namespace std;

  for (const auto &edge : graph->edges()) {
    // Collapse the blue edges
    if (*(edge.data()) == EDGE_TYPE_RED) {
      continue;
    }
    auto from_node = edge.from();
    auto to_node = edge.to();

    graph->collapse_edge( //
        from_node, //
        to_node, //
        [&]( //
            const Eigen::Vector3f &n1,
            const Eigen::Vector3f &n2) {
          return (n1 + n2) * 0.5;
        }, //
        [&]( //
            const Eigen::Vector3f &n1,
            const Eigen::Vector3f &n2,
            const EdgeType &e) {
          return e;
        } //
    );
  }

}