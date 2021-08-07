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
  QuadGraphPtr out_graph = make_shared<animesh::Graph<QuadGraphVertex, EdgeType>>(false);

  // Output all lattice vertices and also map them from surfel ID
  map<string, QuadGraphNodePtr> vertex_to_node;

  // Collect all edges and weed out duplicates.
  std::set<std::pair<std::string, std::string>> counted_edges;
  std::vector<SurfelGraph::Edge> edges;
  for (const auto &edge : graph->edges()) {
    auto &from_surfel = edge.from()->data();
    auto &to_surfel = edge.to()->data();

    if (!from_surfel->is_in_frame(frame_index)) {
      continue;
    }
    if (!to_surfel->is_in_frame(frame_index)) {
      continue;
    }
    if (counted_edges.count({from_surfel->id(), to_surfel->id()}) > 0) {
      continue;
    }
    if (counted_edges.count({to_surfel->id(), from_surfel->id()}) > 0) {
      continue;
    }
    counted_edges.insert({from_surfel->id(), edge.to()->data()->id()});
    edges.push_back(edge);

    // optionally insert both ends of this edge
    Vector3f vertex, tangent, normal;
    if (vertex_to_node.count(from_surfel->id()) == 0) {
      auto offset = from_surfel->reference_lattice_offset();
      from_surfel->get_vertex_tangent_normal_for_frame(frame_index, vertex, tangent, normal);
      auto closest_mesh_vertex = vertex +
          (tangent * offset.x()) +
          ((normal.cross(tangent)) * offset.y());
      auto node = out_graph->add_node({from_surfel->id(),closest_mesh_vertex});
      vertex_to_node.insert({from_surfel->id(), node});
    }
    if (vertex_to_node.count(to_surfel->id()) == 0) {
      auto offset = to_surfel->reference_lattice_offset();
      to_surfel->get_vertex_tangent_normal_for_frame(frame_index, vertex, tangent, normal);
      auto closest_mesh_vertex = vertex +
          (tangent * offset.x()) +
          ((normal.cross(tangent)) * offset.y());
      auto node = out_graph->add_node({to_surfel->id(),closest_mesh_vertex});
      vertex_to_node.insert({to_surfel->id(), node});
    }
  }

  // So edges contains only edges in this frame and each only once
  for (const auto &edge : edges) {
    const auto &from_surfel = edge.from()->data();
    const auto &to_surfel = edge.to()->data();

    Vector3f vertex, tangent, normal;
    from_surfel->get_vertex_tangent_normal_for_frame(frame_index, vertex, tangent, normal);

    Vector3f nbr_vertex, nbr_tangent, nbr_normal;
    to_surfel->get_vertex_tangent_normal_for_frame(frame_index, nbr_vertex, nbr_tangent, nbr_normal);

    // Recompute all the things
    auto best_rot = best_rosy_vector_pair(tangent, normal, nbr_tangent, nbr_normal);

    auto best_shift = best_posy_offset(vertex, best_rot.first, normal,
                                       from_surfel->reference_lattice_offset(),
                                       nbr_vertex, best_rot.second, nbr_normal,
                                       to_surfel->reference_lattice_offset(), rho);

    const auto absDiff = (best_shift.first - best_shift.second).cwiseAbs();
    if (absDiff.maxCoeff() > 1 || (absDiff == Vector2i(1, 1)))
      continue; /* Ignore longer-distance links and diagonal lines for quads */

    spdlog::info("absdiff ({}, {})", absDiff[0], absDiff[1]);
    if (absDiff.sum() != 0) {
      spdlog::info("Adding red edge {}->{} :: t_ij:({},{}) , t_ji:({},{})",
                   from_surfel->id(),
                   to_surfel->id(),
                   best_shift.first.x(),
                   best_shift.first.y(),
                   best_shift.second.x(),
                   best_shift.second.y());
      out_graph->add_edge(
          vertex_to_node.at(from_surfel->id()),
          vertex_to_node.at(to_surfel->id()),
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
            const QuadGraphVertex &n1,
            const QuadGraphVertex &n2) {

          return QuadGraphVertex{(n1.surfel_id + n2.surfel_id), (n1.location + n2.location) * 0.5};
        }, //
        [&]( //
            const QuadGraphVertex &n1,
            const QuadGraphVertex &n2,
            const EdgeType &e) {
          return e;
        } //
    );
  }

}