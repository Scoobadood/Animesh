#pragma once

#include <Eigen/Core>
#include <Graph/Graph.h>
#include "Surfel.h"

class SurfelGraphEdge {
  float m_weight;

  // Best RoSy angles
  unsigned short m_k_low;
  unsigned short m_k_high;

  // PoSy displacements
  Eigen::Vector2i m_t_low;
  Eigen::Vector2i m_t_high;

public:
  explicit SurfelGraphEdge(float weight) //
      : m_weight{weight} //
      , m_k_low{0} //
      , m_k_high{0} //
  {}

  inline unsigned short k_low() const {
    return m_k_low;
  }

  inline unsigned short k_high() const {
    return m_k_high;
  }

  inline void set_k_low(unsigned int k_low) {
    m_k_low = k_low;
  }

  inline void set_k_high(unsigned int k_high) {
    m_k_high = k_high;
  }

  inline const Eigen::Vector2i &t_low() {
    return m_t_low;
  }

  inline const Eigen::Vector2i &t_high() {
    return m_t_high;
  }

  inline void set_t_low(int x, int y) {
    m_t_low = {x, y};
  }

  inline void set_t_high(int x, int y) {
    m_t_high = {x, y};
  }

  inline float weight() const {
    return m_weight;
  }
};

using SurfelGraph = animesh::Graph<std::shared_ptr<Surfel>, SurfelGraphEdge>;
using SurfelGraphPtr = std::shared_ptr<animesh::Graph<std::shared_ptr<Surfel>, SurfelGraphEdge>>;
using SurfelGraphNodePtr = std::shared_ptr<animesh::Graph<std::shared_ptr<Surfel>, SurfelGraphEdge>::GraphNode>;

unsigned int
get_num_frames(const SurfelGraphPtr &surfel_graph);

std::vector<SurfelGraphNodePtr>
get_node_neighbours_in_frame(
    const SurfelGraphPtr &graph,
    const SurfelGraphNodePtr &node_ptr,
    unsigned int frame_index);

std::ostream &operator<<(std::ostream &output, const std::shared_ptr<Surfel> &surfel);

void
set_k(const SurfelGraphPtr &graph,
      const SurfelGraphNodePtr &node1, unsigned short k1,
      const SurfelGraphNodePtr &node2, unsigned short k2);

std::pair<unsigned short, unsigned short>
get_k(const SurfelGraphPtr &graph,
      const SurfelGraphNodePtr &node1,
      const SurfelGraphNodePtr &node2);

void
set_t(const SurfelGraphPtr &graph,
      const SurfelGraphNodePtr &node1, Eigen::Vector2i t1,
      const SurfelGraphNodePtr &node2, Eigen::Vector2i t2);

std::pair<Eigen::Vector2i, Eigen::Vector2i>
get_t(const SurfelGraphPtr &graph,
      const SurfelGraphNodePtr &node1,
      const SurfelGraphNodePtr &node2);
