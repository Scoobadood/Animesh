//
// Created by Dave Durbin (Old) on 2/8/21.
//

#include <vector>
#include <Quad/Quad.h>
#include "quad_geometry_extractor.h"

void
quad_geometry_extractor::extract_geometry(
    std::vector<float> &vertices,
    std::vector<std::pair<std::pair<std::string, unsigned int>, std::pair<std::string, unsigned int>>> &red_edges,
    std::vector<std::pair<std::pair<std::string, unsigned int>, std::pair<std::string, unsigned int>>> &blue_edges,
    std::vector<float> &original_vertex,
    std::vector<float> &vertex_affinity,
    bool rebuild_edge_graph
) {
  using namespace std;

  if( rebuild_edge_graph) {
    m_graph = build_consensus_graph(m_surfel_graph, get_frame(), m_rho);
  }

  vertices.clear();
  red_edges.clear();
  blue_edges.clear();
  original_vertex.clear();
  vertex_affinity.clear();

  if ( m_graph == nullptr) {
    return;
  }
  map<string , unsigned int> node_id_to_vertex_index;
  unsigned int vertex_index = 0;
  for (const auto &node : m_graph->nodes()) {
    node_id_to_vertex_index.insert({node->data().surfel_id, vertex_index});
    float x = node->data().location[0];
    float y = node->data().location[1];
    float z = node->data().location[2];
    vertices.emplace_back(x);
    vertices.emplace_back(y);
    vertices.emplace_back(z);
    vertex_index++;
  }
  for (const auto &edge : m_graph->edges()) {
    unsigned int from = node_id_to_vertex_index.at(edge.from()->data().surfel_id);
    unsigned int to = node_id_to_vertex_index.at(edge.to()->data().surfel_id);

    if (*edge.data() == EDGE_TYPE_RED) {
      red_edges.emplace_back(
          make_pair(
              make_pair(edge.from()->data().surfel_id, from),
              make_pair(edge.to()->data().surfel_id, to)));
    } else {
      blue_edges.emplace_back(
          make_pair(
              make_pair(edge.from()->data().surfel_id, from),
              make_pair(edge.to()->data().surfel_id, to)));
    }
  }
  for( const auto & node : m_surfel_graph->nodes()) {
    if( !node->data()->is_in_frame(0)) {
      continue;
    }
    Eigen::Vector3f v, t, n;
    node->data()->get_vertex_tangent_normal_for_frame(0, v, t, n);
    original_vertex.emplace_back(v[0]);
    original_vertex.emplace_back(v[1]);
    original_vertex.emplace_back(v[2]);
    auto vi = node_id_to_vertex_index[node->data()->id()];
    vertex_affinity.emplace_back(vertices[vi * 3 + 0]);
    vertex_affinity.emplace_back(vertices[vi * 3 + 1]);
    vertex_affinity.emplace_back(vertices[vi * 3 + 2]);
  }
}