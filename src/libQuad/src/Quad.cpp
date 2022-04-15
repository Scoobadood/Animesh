//
// Created by Dave Durbin (Old) on 12/7/21.
//

#include "Quad.h"

#include <PoSy/PoSy.h>
#include <Surfel/SurfelGraph.h>
#include <Eigen/Core>
#include <Eigen/Geometry>
#include <map>
#include <queue>

std::vector<SurfelGraph::Edge>
get_edges_in_frame(const SurfelGraphPtr &graph, int frame_index) {
  using namespace std;

  set<pair<string, string>> checked_edges;
  vector<SurfelGraph::Edge> edges_in_frame;

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
    if ((checked_edges.count({from_id, to_id}) == 1) || (checked_edges.count({to_id, from_id}) == 1)) {
      continue;
    }

    // Otherwise add this to the list of edges we'll include in the graph
    checked_edges.emplace(make_pair(from_id, to_id));
    edges_in_frame.emplace_back(edge);
  }
  return edges_in_frame;
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

ConsensusGraphNodePtr
add_or_retrieve_node(const ConsensusGraphPtr &out_graph,
                     std::map<std::string,
                              ConsensusGraphNodePtr> &output_graph_nodes_by_surfel_id,
                     unsigned int frame_idx,
                     const std::shared_ptr<Surfel> &surfel,
                     float rho) {
  ConsensusGraphNodePtr node;
  std::string surfel_id = surfel->id();
  if (output_graph_nodes_by_surfel_id.count(surfel_id) == 0) {
    ConsensusGraphVertex cgv;
    cgv.surfel_id = surfel_id;
    cgv.location = surfel->reference_lattice_vertex_in_frame(frame_idx, rho);
    Eigen::Vector3f ignored1, ignored2, normal;
    surfel->get_vertex_tangent_normal_for_frame(frame_idx, ignored1, ignored2, normal);
    cgv.normal = normal;
    node = out_graph->add_node(cgv);
    output_graph_nodes_by_surfel_id.emplace(surfel_id, node);
  } else {
    node = output_graph_nodes_by_surfel_id[surfel_id];
  }
  return node;
}

ConsensusGraphPtr
build_consensus_graph(const SurfelGraphPtr &graph, int frame_index, float rho) {
  using namespace Eigen;
  using namespace std;

  spdlog::info("Building consensus graph from graph with {} nodes and {} edges",
               graph->num_nodes(),
               graph->num_edges());

  // Make the output graph
  ConsensusGraphPtr out_graph = make_shared<animesh::Graph<ConsensusGraphVertex, EdgeType>>(false);

  // For each edge with end nodes in this frame
  auto edges = get_edges_in_frame(graph, frame_index);
  spdlog::info("  Found {} unique edges", edges.size());

  // For each edge in this list, insert the end vertices if not present, then add the edge.
  map<string, ConsensusGraphNodePtr> output_graph_nodes_by_surfel_id;
  for (const auto &edge: edges) {
    // Create a node for the from_node if there's not one already.
    const auto &from_surfel = edge.from()->data();
    const auto &to_surfel = edge.to()->data();
    const auto from_node = add_or_retrieve_node(out_graph,
                                                output_graph_nodes_by_surfel_id,
                                                frame_index,
                                                from_surfel,
                                                rho);
    const auto to_node = add_or_retrieve_node(out_graph,
                                              output_graph_nodes_by_surfel_id,
                                              frame_index,
                                              to_surfel,
                                              rho);
    auto ts = get_t(graph, edge.from(), edge.to());
    auto t_ij = ts.first;
    auto t_ji = ts.second;
    auto edgeType = compute_edge_type(t_ij, t_ji);
    if (edgeType == EDGE_TYPE_NON) {
      spdlog::info("  skipped edge from {} {}", from_surfel->id(), to_surfel->id());
      continue;
    }
    spdlog::info("  adding {} edge {}->{} :: t_ij:({},{}) , t_ji:({},{}, length: {})",
                 edgeType == EDGE_TYPE_BLU ? "blue" : "red",
                 from_surfel->id(),
                 to_surfel->id(),
                 t_ij[0], t_ij[1],
                 t_ji[0], t_ji[1],
                 (from_node->data().location - to_node->data().location).norm());

    out_graph->add_edge(from_node, to_node, edgeType);
  }
  spdlog::info("New graph has {} edges", out_graph->edges().size());
  return out_graph;
}

void
collapse(const ConsensusGraphPtr &graph) {
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
      const ConsensusGraphVertex &n1,
      float weight1,
      const ConsensusGraphVertex &n2,
      float weight2) {

    return ConsensusGraphVertex{(n1.surfel_id + n2.surfel_id),
                                ((n1.location * weight1) + (n2.location * weight2)) / (weight1 + weight2),
                                ((n1.normal * weight1) + (n2.normal * weight2)).normalized()
    };
  };

  auto edge_sort_pred = [](
      const pair<ConsensusGraphNodePtr, ConsensusGraphNodePtr> &e1,
      const pair<ConsensusGraphNodePtr, ConsensusGraphNodePtr> &e2
  ) -> bool {
    auto l1 = (e2.second->data().location - e2.first->data().location).squaredNorm();
    auto l2 = (e1.second->data().location - e1.first->data().location).squaredNorm();
    // Using this on a queue so the FIRST thing should be the smallest
    return l1 < l2;
  };

  // Extract blue edges
  priority_queue<pair<ConsensusGraphNodePtr, ConsensusGraphNodePtr>,
                 vector<pair<ConsensusGraphNodePtr, ConsensusGraphNodePtr>>,
                 auto (*)(const pair<ConsensusGraphNodePtr, ConsensusGraphNodePtr> &,
                          const pair<ConsensusGraphNodePtr, ConsensusGraphNodePtr> &)->bool>
      blue_edges{edge_sort_pred};

  for (const auto &edge: graph->edges()) {
    if (*(edge.data()) == EDGE_TYPE_BLU) {
      blue_edges.emplace(edge.from(), edge.to());
    }
  }

  // Iterate over the list, deleting edges and adding in newly generated ones
  while (!blue_edges.empty()) {
    auto next_edge = blue_edges.top();
    blue_edges.pop();
    auto from_node = next_edge.first;
    auto to_node = next_edge.second;

    // Check that the edge is still present as it may not be
    vector<ConsensusGraph::Edge> removed_edges;
    vector<ConsensusGraph::Edge> created_edges;
    if (graph->has_edge(from_node, to_node)) {
      graph->collapse_edge(from_node, to_node,
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

void
extract_face(const ConsensusGraphPtr &graph,
              std::vector<Eigen::Vector3f> &vertices,
              std::vector<Eigen::Vector3f> &vertex_normals,
              std::vector<std::vector<unsigned long>> &faces
) {
}

void
extract_faces(const ConsensusGraphPtr &graph,
              std::vector<Eigen::Vector3f> &vertices,
              std::vector<Eigen::Vector3f> &vertex_normals,
              std::vector<std::vector<unsigned long>> &faces
) {
  using namespace std;

  map<string, unsigned long> vertex_name_to_index;
  set<pair<string, string>> used_edges;

  for (const auto &edge: graph->edges()) {
    if (used_edges.count({edge.from()->data().surfel_id,edge.to()->data().surfel_id}) > 0) {
      continue;
    }

    auto starting_node = edge.from();

    // Get possible next edges from 'to'
    bool found_face = false;
    vector<ConsensusGraph::Edge> face;
    face.emplace_back(edge);

    while (true) {
      auto curr_node = face.back().to();
      auto curr_surfel = curr_node->data();
      auto curr_surfel_id = curr_surfel.surfel_id;
      auto prev_node = face.back().from();
      auto prev_surfel = prev_node->data();
      auto prev_surfel_id = prev_surfel.surfel_id;

      auto last_edge_vector = curr_surfel.location - prev_surfel.location;

      // Get next edges and see if we can make a quad from them.
      auto potential_next_edges = graph->edges_from(curr_node, prev_node);
      bool added_edge = false;
      for (const auto &pne: potential_next_edges) {
        assert(pne.from() == curr_node);
        auto next_surfel = pne.to()->data();
        auto next_surfel_id = next_surfel.surfel_id;
        if (used_edges.count({curr_surfel_id, next_surfel_id}) > 0) {
          continue;
        }
        auto next_edge_vector = next_surfel.location - curr_surfel.location;
        // compute triple product
        auto triple = last_edge_vector.cross(next_edge_vector).dot(curr_surfel.normal);
        if (triple > 0) {
          // Add to tentative face
          face.emplace_back(pne);
          added_edge = true;
          break;
        }
      }

      // We considered all edges coming from this one and didn;t find an appropriat edge.
      // Skip it.
      if (!added_edge) {
        break;
      }

      // If we've closed a cycle and it's the right length, found_face
      if (face.size() == 3) {
        // Was loop closed?
        if (face.back().to() == starting_node) {
          found_face = true;
          break;
        }
      } else if (face.size() == 4 ) {
        // Was loop closed?
        if (face.back().to() == starting_node) {
          found_face = true;
        }
        break;
      }
    }


    // TEST
    // END TEST

    if (found_face) {
      // Mark all used as used.
      // Add to faces
      // Mark each edge as complete
      faces.emplace_back();
      for (const auto &face_edge: face) {
        const auto & from_surfel = face_edge.from()->data();
        const auto & from_surfel_id = from_surfel.surfel_id;
        used_edges.emplace(make_pair(from_surfel_id, face_edge.to()->data().surfel_id));
        if (vertex_name_to_index.count(from_surfel_id) == 0) {
          vertices.emplace_back(from_surfel.location);
          vertex_normals.emplace_back(from_surfel.normal);
          vertex_name_to_index.emplace(from_surfel_id, vertices.size());
        }
        faces.back().emplace_back(vertex_name_to_index[from_surfel_id]);
      }
    }
  }


  for (const auto &edge: graph->edges()) {
    if (used_edges.count({edge.to()->data().surfel_id,edge.from()->data().surfel_id}) > 0) {
      continue;
    }

    auto starting_node = edge.to();

    // Get possible next edges from 'to'
    bool found_face = false;
    vector<ConsensusGraph::Edge> face;
    face.emplace_back(ConsensusGraph::Edge(edge.to(), edge.from(), edge.data()));

    while (true) {
      auto curr_node = face.back().to();
      auto curr_surfel = curr_node->data();
      auto curr_surfel_id = curr_surfel.surfel_id;
      auto prev_node = face.back().from();
      auto prev_surfel = prev_node->data();
      auto prev_surfel_id = prev_surfel.surfel_id;

      auto last_edge_vector = curr_surfel.location - prev_surfel.location;

      // Get next edges and see if we can make a quad from them.
      auto potential_next_edges = graph->edges_from(curr_node, prev_node);
      bool added_edge = false;
      for (const auto &pne: potential_next_edges) {
        assert(pne.from() == curr_node);
        auto next_surfel = pne.to()->data();
        auto next_surfel_id = next_surfel.surfel_id;
        if (used_edges.count({curr_surfel_id, next_surfel_id}) > 0) {
          continue;
        }
        auto next_edge_vector = next_surfel.location - curr_surfel.location;
        // compute triple product
        auto triple = last_edge_vector.cross(next_edge_vector).dot(curr_surfel.normal);
        if (triple > 0) {
          // Add to tentative face
          face.emplace_back(pne);
          added_edge = true;
          break;
        }
      }

      // We considered all edges coming from this one and didn;t find an appropriat edge.
      // Skip it.
      if (!added_edge) {
        break;
      }

      // If we've closed a cycle and it's the right length, found_face
      if (face.size() == 3) {
        // Was loop closed?
        if (face.back().to() == starting_node) {
          found_face = true;
          break;
        }
      } else if (face.size() == 4 ) {
        // Was loop closed?
        if (face.back().to() == starting_node) {
          found_face = true;
        }
        break;
      }
    }


    // TEST
    // END TEST

    if (found_face) {
      // Mark all used as used.
      // Add to faces
      // Mark each edge as complete
      faces.emplace_back();
      for (const auto &face_edge: face) {
        const auto & from_surfel = face_edge.from()->data();
        const auto & from_surfel_id = from_surfel.surfel_id;
        used_edges.emplace(make_pair(from_surfel_id, face_edge.to()->data().surfel_id));
        if (vertex_name_to_index.count(from_surfel_id) == 0) {
          vertices.emplace_back(from_surfel.location);
          vertex_normals.emplace_back(from_surfel.normal);
          vertex_name_to_index.emplace(from_surfel_id, vertices.size());
        }
        faces.back().emplace_back(vertex_name_to_index[from_surfel_id]);
      }
    }
  }
}