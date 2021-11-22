//
// Created by Dave Durbin (Old) on 12/7/21.
//

#include "Quad.h"

#include <PoSy/PoSy.h>
#include <Surfel/SurfelGraph.h>
#include <map>
#include <queue>
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

  const string &surfel_id = surfel_ptr->id();

  // If it's in our output map, return early with the map's value
  const auto found_iter = out_nodes_by_surfel_id.find(surfel_id);
  if (found_iter != out_nodes_by_surfel_id.end()) {
    spdlog::debug("   Ignored insert for node {}", surfel_id);
    return found_iter->second;
  }

  auto offset = surfel_ptr->reference_lattice_offset();
  Vector3f vertex, tangent, normal;
  surfel_ptr->get_vertex_tangent_normal_for_frame(frame_index, vertex, tangent, normal);
  Vector3f orth_tangent = normal.cross(tangent);
  auto lattice_position = vertex +
      (tangent * offset[0]) +
      (orth_tangent * offset[1]);
  spdlog::debug("Inserted node {} at ({}, {}, {})",
                surfel_id,
                lattice_position[0],
                lattice_position[1],
                lattice_position[2]);

  auto node = out_graph->add_node({surfel_id, lattice_position});
  out_nodes_by_surfel_id.insert({surfel_id, node});
  return node;
}

std::map<std::pair<std::string, std::string>, SurfelGraph::Edge>
get_unique_edges_in_frame(const SurfelGraphPtr graph, int frame_index) {
  using namespace std;

  map<pair<string, string>, SurfelGraph::Edge> included_edges;
  for (const auto &edge: graph->edges()) {
    const auto from_surfel = edge.from()->data();
    const auto to_surfel = edge.to()->data();
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
  return included_edges;
}

EdgeType compute_edge_type(const Eigen::Vector2i &t_ij,
                           const Eigen::Vector2i &t_ji) {
  const auto manhattanEdgeLength = (t_ij - t_ji).cwiseAbs().sum();
  if (manhattanEdgeLength >= 2) {
    return EDGE_TYPE_NON;
  }

  if (manhattanEdgeLength == 1) {
    return EDGE_TYPE_RED;
  } else // manhattanEdgeLength == 0
  {
    return EDGE_TYPE_BLU;
  }
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
  auto included_edges = get_unique_edges_in_frame(graph, frame_index);
  spdlog::debug("New graph has potential {} edges from old graph {}", included_edges.size(), graph->edges().size());

  // For each edge in this list, insert the end vertices if not present, then add the edge.
  map<string, QuadGraphNodePtr> output_graph_nodes_by_surfel_id;
  for (const auto &edge_entry: included_edges) {
    const auto edge = edge_entry.second;
    const auto from_surfel_ptr = edge.from()->data();
    const auto from_node = maybe_insert_node_in_graph(from_surfel_ptr, frame_index,
                                                      out_graph, output_graph_nodes_by_surfel_id);

    const auto to_surfel_ptr = edge.to()->data();
    const auto to_node = maybe_insert_node_in_graph(to_surfel_ptr, frame_index,
                                                    out_graph, output_graph_nodes_by_surfel_id);

    auto t = get_t(graph, edge.from(), edge.to());
    auto t_ij = t.first;
    auto t_ji = t.second;
    EdgeType edgeType = compute_edge_type(t_ij, t_ji);
    if (edgeType == EDGE_TYPE_NON) {
      spdlog::debug("Skipped edge from {} {}",
                   from_surfel_ptr->id(),
                   to_surfel_ptr->id()
      );
      continue;
    }

    spdlog::debug("Adding {} edge {}->{} :: t_ij:({},{}) , t_ji:({},{}, length: {})",
                  edgeType == EDGE_TYPE_BLU ? "blue" : "red",
                  from_surfel_ptr->id(),
                  to_surfel_ptr->id(),
                  t_ij[0], t_ij[1],
                  t_ji[0], t_ji[1],
                  (from_node->data().location - to_node->data().location).norm());

    out_graph->add_edge(from_node, to_node, edgeType);
  }
  spdlog::debug("New graph has actual {} edges", out_graph->edges().size());

  // No edge added
  return out_graph;
}

void
collapse(const QuadGraphPtr &graph,
         float rho
) {
  using namespace std;

  auto edge_merge_function = [&]( //
      const EdgeType &e1,
      float weight1,
      const EdgeType &e2,
      float weight2) {
    // TODO : Need to be smarter here
    if (e1 == EDGE_TYPE_RED)
      return e1;
    if (e2 == EDGE_TYPE_RED)
      return e2;
    return e1;
  };

  auto node_merge_function = [&]( //
      const QuadGraphVertex &n1,
      float weight1,
      const QuadGraphVertex &n2,
      float weight2) {

    return QuadGraphVertex{(n1.surfel_id + n2.surfel_id),
                           ((n1.location * weight1) + (n2.location * weight2)) / (weight1 + weight2)};
  };

  auto edge_sort_pred = [](
      const pair<QuadGraphNodePtr, QuadGraphNodePtr> &e1,
      const pair<QuadGraphNodePtr, QuadGraphNodePtr> &e2
  ) -> bool {
    auto l1 = (e2.second->data().location - e2.first->data().location).squaredNorm();
    auto l2 = (e1.second->data().location - e1.first->data().location).squaredNorm();
    // Using this on a queue so the FIRST thing should be the smallest
    return l1 < l2;
  };

  // Extract blue edges
  priority_queue<pair<QuadGraphNodePtr, QuadGraphNodePtr>,
                 vector<pair<QuadGraphNodePtr, QuadGraphNodePtr>>,
                 auto (*)(const pair<QuadGraphNodePtr, QuadGraphNodePtr> &,
                          const pair<QuadGraphNodePtr, QuadGraphNodePtr> &)->bool> blue_edges{edge_sort_pred};

  for (const auto &edge: graph->edges()) {
    // Collapse the blue edges
    if (*(edge.data()) == EDGE_TYPE_BLU) {
      blue_edges.emplace(edge.from(), edge.to());
    }
  }

  // Iterate over the list, deleting edges and adding in newly generated ones
  while (!blue_edges.empty()) {
    cout << blue_edges.size() << endl;
    auto next = blue_edges.top();
    blue_edges.pop();
    auto from_node = next.first;
    auto to_node = next.second;

    // Check that the edge is still present as it may not be
    vector<QuadGraph::Edge> removed_edges;
    vector<QuadGraph::Edge> created_edges;
    if (graph->has_edge(from_node, to_node)) {
      graph->collapse_edge(from_node, to_node,
                           node_merge_function,
                           edge_merge_function,
                           removed_edges,
                           created_edges);
    } else if (graph->has_edge(to_node, from_node)) {
      graph->collapse_edge(to_node, from_node,
                           node_merge_function,
                           edge_merge_function,
                           removed_edges,
                           created_edges);
    }
    // Add newly created edges to queue
    for (const auto &c_edge: created_edges) {
      if (*(c_edge.data()) == EDGE_TYPE_BLU) {
        blue_edges.emplace(c_edge.from(), c_edge.to());
      }
    }
  }
}