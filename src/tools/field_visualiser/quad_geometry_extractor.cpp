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
    std::vector<std::pair<std::pair<std::string, unsigned int>, std::pair<std::string, unsigned int>>> &blue_edges
) {
  using namespace std;

  vertices.clear();
  red_edges.clear();
  blue_edges.clear();
  if (m_graph == nullptr) {
    return;
  }
  map<string, unsigned int> node_id_to_vertex_index;
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
    string from_vertex_name = edge.from()->data().surfel_id;
    string to_vertex_name = edge.to()->data().surfel_id;

    unsigned int from_vertex_idx = node_id_to_vertex_index.at(from_vertex_name);
    unsigned int to_vertex_idx = node_id_to_vertex_index.at(to_vertex_name);

    if (EDGE_TYPE_RED == *(edge.data())) {
      red_edges.emplace_back(
          make_pair(
              make_pair(from_vertex_name, from_vertex_idx),
              make_pair(to_vertex_name, to_vertex_idx)));
    } else if (EDGE_TYPE_BLU == *(edge.data())) {
      blue_edges.emplace_back(
          make_pair(
              make_pair(from_vertex_name, from_vertex_idx),
              make_pair(to_vertex_name, to_vertex_idx)));
    } else {
      spdlog::error("Edge with no type");
    }
  }
}