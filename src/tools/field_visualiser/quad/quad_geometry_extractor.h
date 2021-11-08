//
// Created by Dave Durbin (Old) on 2/8/21.
//

#ifndef ANIMESH_TOOLS_FIELD_VISUALISER_QUAD_GEOMETRY_EXTRACTOR_H
#define ANIMESH_TOOLS_FIELD_VISUALISER_QUAD_GEOMETRY_EXTRACTOR_H

#include <Quad/Quad.h>
#include <Surfel/SurfelGraph.h>
#include "geometry_extractor.h"
class quad_geometry_extractor : public geometry_extractor {
public:
  explicit quad_geometry_extractor(float rho) //
    : geometry_extractor() //
    , m_rho{rho} //
    , m_graph{nullptr} {}

  void extract_geometry(
      std::vector<float> &vertices,
      std::vector<std::pair<std::pair<std::string, unsigned int>, std::pair<std::string, unsigned int>>> &red_edges,
      std::vector<std::pair<std::pair<std::string, unsigned int>, std::pair<std::string, unsigned int>>> &blue_edges,
      std::vector<float>& original_vertex,
      std::vector<float>& vertex_affinity,
      bool rebuild_edge_graph = true
  );

  void set_graph(const SurfelGraphPtr &graph) {
    using namespace std;
    m_red_edges.clear();
    m_blue_edges.clear();
    m_surfel_graph = graph;
  }

  void collapse() {
      ::collapse(m_graph, m_rho);
  }

private:
  float m_rho;
  QuadGraphPtr m_graph;
  SurfelGraphPtr m_surfel_graph;
  std::vector<std::pair<std::string,std::string>> m_red_edges;
  std::vector<std::pair<std::string,std::string>> m_blue_edges;

};

#endif //ANIMESH_TOOLS_FIELD_VISUALISER_QUAD_GEOMETRY_EXTRACTOR_H
