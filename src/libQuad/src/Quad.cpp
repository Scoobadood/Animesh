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
maybe_insert_vertex_in_graph(
    unsigned int frame_index,
    std::map<std::string, QuadGraphNodePtr> &vertex_to_node,
    const std::shared_ptr<Surfel> &surfel,
    const QuadGraphPtr &out_graph
) {
  if (vertex_to_node.count(surfel->id()) == 0) {
    auto offset = surfel->reference_lattice_offset();
    Eigen::Vector3f vertex, tangent, normal;
    surfel->get_vertex_tangent_normal_for_frame(frame_index, vertex, tangent, normal);
    auto closest_mesh_vertex = vertex +
        (tangent * offset.x()) +
        ((normal.cross(tangent)) * offset.y());
    auto node = out_graph->add_node({surfel->id(), closest_mesh_vertex});
    vertex_to_node.insert({surfel->id(), node});
  }
}

// Collate the set of unique edges
void collate_unique_edges(const SurfelGraphPtr &graph,
                          unsigned int frame_index,
                          std::map<std::string, QuadGraphNodePtr>& vertex_to_node,
                          std::set<std::pair<std::string, std::string>>& counted_edges,
                          std::vector<SurfelGraph::Edge>& edges,
                          QuadGraphPtr out_graph

) {
  for (const auto &edge : graph->edges()) {
    const auto &from_surfel = edge.from()->data();
    const auto &to_surfel = edge.to()->data();

    // skip edge if either vertex is not in frame
    if (!from_surfel->is_in_frame(frame_index)) {
      continue;
    }
    if (!to_surfel->is_in_frame(frame_index)) {
      continue;
    }
    // Skip edge if we've seen it (or its inverse)
    if (counted_edges.count({from_surfel->id(), to_surfel->id()}) > 0) {
      continue;
    }
    if (counted_edges.count({to_surfel->id(), from_surfel->id()}) > 0) {
      continue;
    }
    // Note that we've seen this edge
    counted_edges.insert({from_surfel->id(), to_surfel->id()});
    edges.push_back(edge);

    // optionally insert both ends of this edge
    maybe_insert_vertex_in_graph(frame_index, vertex_to_node, from_surfel, out_graph);
    maybe_insert_vertex_in_graph(frame_index, vertex_to_node, to_surfel, out_graph);
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
  QuadGraphPtr out_graph = make_shared<animesh::Graph<QuadGraphVertex, EdgeType>>(false);
  map<string, QuadGraphNodePtr> vertex_to_node;
  set<pair<string, string>> counted_edges;
  vector<SurfelGraph::Edge> edges;
  collate_unique_edges(graph, frame_index, vertex_to_node, counted_edges, edges, out_graph);

  // Now edges contains only edges in this frame and each only once
  for (const auto &edge : edges) {
    const auto from_surfel = edge.from()->data();
    const auto to_surfel = edge.to()->data();

    Vector3f vertex, tangent, normal;
    from_surfel->get_vertex_tangent_normal_for_frame(frame_index, vertex, tangent, normal);

    Vector3f nbr_vertex, nbr_tangent, nbr_normal;
    to_surfel->get_vertex_tangent_normal_for_frame(frame_index, nbr_vertex, nbr_tangent, nbr_normal);

    // TODO: Could just do the k_ij, t_ij thing here.
    // Recompute all the things
    auto best_rot = best_rosy_vector_pair(tangent, normal, nbr_tangent, nbr_normal);
    //FIXME: The offset is only valid in the regular frame n'est-ce pas?
    // Indeed this is true! So this best shift is wrong. So we should use t_ij and t_ji
    auto best_shift = best_posy_offset(vertex, best_rot.first, normal,
                                       from_surfel->reference_lattice_offset(),
                                       nbr_vertex, best_rot.second, nbr_normal,
                                       to_surfel->reference_lattice_offset(), rho);

    const int manhattanLength = (best_shift.first - best_shift.second).cwiseAbs().sum();
    std::string colour;
    if (manhattanLength == 0) {
      // These are the same vertex
      spdlog::info("Adding blue edge {}->{} :: t_ij:({},{}) , t_ji:({},{})",
                   from_surfel->id(),
                   to_surfel->id(),
                   best_shift.first.x(),
                   best_shift.first.y(),
                   best_shift.second.x(),
                   best_shift.second.y());
      out_graph->add_edge(
          vertex_to_node.at(from_surfel->id()),
          vertex_to_node.at(to_surfel->id()),
          EDGE_TYPE_BLU
      );

    } else if (manhattanLength == 1) {
      // These are an edge
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