//
// Created by Dave Durbin (Old) on 2/8/21.
//

#include <vector>
#include <Quad/Quad.h>
#include "quad_geometry_extractor.h"

void
quad_geometry_extractor::extract_geometry(
    const SurfelGraphPtr &graph_ptr,
    std::vector<float> &vertices,
    std::vector<std::pair<unsigned int, unsigned int>> &red_edges,
    std::vector<std::pair<unsigned int, unsigned int>> &blue_edges
    ) {
  using namespace std;

  vertices.clear();
  red_edges.clear();
  blue_edges.clear();
  if ( m_graph == nullptr) {
    return;
  }
  map<const QuadGraphNodePtr , unsigned int> node_id_to_vertex_index;
  unsigned int vertex_index = 0;
  for (const auto &node : m_graph->nodes()) {
    node_id_to_vertex_index.insert({node, vertex_index});
    float x = node->data().x();
    float y = node->data().y();
    float z = node->data().z();
    vertices.emplace_back(x);
    vertices.emplace_back(y);
    vertices.emplace_back(z);
    vertex_index++;
  }
  for (const auto &edge : m_graph->edges()) {
    unsigned int from = node_id_to_vertex_index.at(edge.from());
    unsigned int to = node_id_to_vertex_index.at(edge.to());

    if (*edge.data() == EDGE_TYPE_RED) {
      red_edges.emplace_back(make_pair(from, to));
    } else {
      blue_edges.emplace_back(make_pair(from, to));
    }
  }
}