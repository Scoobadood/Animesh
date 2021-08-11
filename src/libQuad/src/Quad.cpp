//
// Created by Dave Durbin (Old) on 12/7/21.
//

#include "Quad.h"

#include <PoSy/PoSy.h>
#include <RoSy/RoSy.h>
#include <Surfel/SurfelGraph.h>
#include <map>
#include <Eigen/Geometry>

void
maybe_insert_node_in_graph( //
    const std::shared_ptr<Surfel> &surfel_ptr, //
    unsigned int frame_index, //
    const QuadGraphPtr &out_graph,
    std::map<std::string, QuadGraphNodePtr> &out_nodes_by_surfel_id //
) {
  using namespace Eigen;
  using namespace std;

  string surfel_id = surfel_ptr->id();
  // If it's alreay there, return early.
  if (out_nodes_by_surfel_id.count(surfel_id) > 0) {
    return;
  }

  auto offset = surfel_ptr->reference_lattice_offset();
  Vector3f vertex, tangent, normal;
  surfel_ptr->get_vertex_tangent_normal_for_frame(frame_index, vertex, tangent, normal);
  auto closest_mesh_vertex = vertex +
      (tangent * offset.x()) +
      ((normal.cross(tangent)) * offset.y());
  auto node = out_graph->add_node({surfel_id, closest_mesh_vertex});
  out_nodes_by_surfel_id.insert({surfel_id, node});
}

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
  map<string, QuadGraphNodePtr> out_nodes_by_surfel_id;

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
    maybe_insert_node_in_graph(from_surfel, frame_index, out_graph, out_nodes_by_surfel_id);
    maybe_insert_node_in_graph(to_surfel, frame_index, out_graph, out_nodes_by_surfel_id);
  }

  // So edges contains only edges in this frame and each only once
  for (const auto &edge : edges) {
    auto t_ij = edge.data()->t_ij(frame_index);
    auto t_ji = edge.data()->t_ji(frame_index);

    const auto manhattanEdgeLength = (t_ij - t_ji).cwiseAbs().sum();
    if (manhattanEdgeLength < 2) {
      EdgeType type;
      const auto &from_surfel_id = edge.from()->data()->id();
      const auto &to_surfel_id = edge.to()->data()->id();
      if (manhattanEdgeLength == 1) {
        type = EDGE_TYPE_RED;
      } else // manhattanEdgeLength == 0
      {
        type = EDGE_TYPE_BLU;
      }
      spdlog::info("Adding {} edge {}->{} :: t_ij:({},{}) , t_ji:({},{})",
                   type == EDGE_TYPE_BLU ? "blue" : "red",
                   from_surfel_id,
                   to_surfel_id,
                   t_ij[0], t_ij[1],
                   t_ji[0], t_ji[1]);

      out_graph->add_edge(
          out_nodes_by_surfel_id.at(from_surfel_id),
          out_nodes_by_surfel_id.at(to_surfel_id),
          type
      );
    }
    // No edge added
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