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
      const SurfelGraphPtr &graph_ptr,
      std::vector<float> &vertices,
      std::vector<std::pair<unsigned int, unsigned int>> &red_edges,
      std::vector<std::pair<unsigned int, unsigned int>> &blue_edges
  );

  void set_graph(const SurfelGraphPtr &graph) {
    m_graph = build_edge_graph(get_frame(), graph, m_rho);
  }

  void collapse() {
      ::collapse(0, m_graph, m_rho);
  }

private:
  float m_rho;
  QuadGraphPtr m_graph;
};

#endif //ANIMESH_TOOLS_FIELD_VISUALISER_QUAD_GEOMETRY_EXTRACTOR_H
