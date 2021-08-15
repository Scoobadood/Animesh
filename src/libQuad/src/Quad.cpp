//
// Created by Dave Durbin (Old) on 12/7/21.
//

#include "Quad.h"

#include <PoSy/PoSy.h>
#include <Surfel/SurfelGraph.h>
#include <map>
#include <Eigen/Geometry>

QuadGraphNodePtr
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
  auto iter = out_nodes_by_surfel_id.find(surfel_id);
  if (iter != out_nodes_by_surfel_id.end()) {
    spdlog::info("   Ignored insert for node {}", surfel_id);
    return iter->second;
  }

  auto offset = surfel_ptr->reference_lattice_offset();
  Vector3f vertex, tangent, normal;
  surfel_ptr->get_vertex_tangent_normal_for_frame(frame_index, vertex, tangent, normal);
  auto lattice_position = vertex +
      (tangent * offset[0]) +
      ((normal.cross(tangent)) * offset[1]);
  spdlog::info("Inserted node {} at ({}, {}, {})",
               surfel_id,
               lattice_position[0],
               lattice_position[1],
               lattice_position[2]);

  auto node = out_graph->add_node({surfel_id, lattice_position});
  out_nodes_by_surfel_id.insert({surfel_id, node});
  return node;
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
  QuadGraphPtr out_graph = make_shared<animesh::Graph<QuadGraphVertex, EdgeType>>(true);

  // Collect all edges, weeding out duplicates.
  map<pair<string, string>, SurfelGraph::Edge> included_edges;
  for (const auto &edge : graph->edges()) {
    const auto &from_surfel = edge.from()->data();
    const auto &to_surfel = edge.to()->data();

    // Skip edges where one end is not in this frame
    if (!((from_surfel->is_in_frame(frame_index)) && to_surfel->is_in_frame(frame_index))) {
      continue;
    }

    // If we already considered this edge (usually its inverse), skip it
    const string from_id = from_surfel->id();
    const string to_id = to_surfel->id();
    if ((included_edges.count({from_id, to_id}) == 1) || (included_edges.count({to_id, from_id}) == 1)) {
      continue;
    }

    // Otherwise add this to the list of edges we'll include in the graph
    included_edges.emplace(make_pair(from_id, to_id), edge);
  }
  spdlog::info("New graph has potential {} edges from old graph {}", included_edges.size(), graph->edges().size());

  // For each edge in this list, insert the end vertices if not present, then add the edge.
  map<string, QuadGraphNodePtr> output_graph_nodes_by_surfel_id;
  for (const auto &edge_entry : included_edges) {
    const auto edge = edge_entry.second;

    const auto from_node = maybe_insert_node_in_graph(edge.from()->data(), frame_index,
                                                      out_graph, output_graph_nodes_by_surfel_id);
    const auto to_node = maybe_insert_node_in_graph(edge.to()->data(), frame_index,
                                                    out_graph, output_graph_nodes_by_surfel_id);

    auto t_ij = edge.data()->t_ij(frame_index);
    auto t_ji = edge.data()->t_ji(frame_index);
    const auto manhattanEdgeLength = (t_ij - t_ji).cwiseAbs().sum();
    if (manhattanEdgeLength >= 2) {
      spdlog::info("Skipped edge from {} {}",
                   edge.from()->data()->id(),
                   edge.to()->data()->id()
      );
      continue;
    }

    EdgeType type;
    const auto &from_surfel_id = edge.from()->data()->id();
    const auto &to_surfel_id = edge.to()->data()->id();
    if (manhattanEdgeLength == 1) {
      type = EDGE_TYPE_RED;
    } else // manhattanEdgeLength == 0
    {
      type = EDGE_TYPE_BLU;
    }
    spdlog::info("Adding {} edge {}->{} :: t_ij:({},{}) , t_ji:({},{}, length: {})",
                 type == EDGE_TYPE_BLU ? "blue" : "red",
                 from_surfel_id,
                 to_surfel_id,
                 t_ij[0], t_ij[1],
                 t_ji[0], t_ji[1],
                 (from_node->data().location - to_node->data().location).norm());

    out_graph->add_edge(from_node, to_node, type);
  }
  spdlog::info("New graph has actual {} edges", out_graph->edges().size());

  // No edge added
  return out_graph;
}

void
collapse(int frame_index,
         const QuadGraphPtr &graph,
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