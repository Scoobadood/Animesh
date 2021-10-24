//
// Created by Dave Durbin (Old) on 4/7/21.
//
#include "SurfelGraph.h"

std::vector<SurfelGraphNodePtr>
get_node_neighbours_in_frame(
    const SurfelGraphPtr &graph,
    const SurfelGraphNodePtr &node_ptr,
    unsigned int frame_index) {
  std::vector<SurfelGraphNodePtr> neighbours_in_frame;
  for (const auto &neighbour_node: graph->neighbours(node_ptr)) {
    if (neighbour_node->data()->is_in_frame(frame_index)) {
      neighbours_in_frame.emplace_back(neighbour_node);
    }
  }
  return neighbours_in_frame;
}

/**
 * @param surfel_graph The graph.
 * @return the number of frames spanned by this graph.
 */
unsigned int
get_num_frames(const SurfelGraphPtr &surfel_graph) {
  // Compute the number of frames
  unsigned int max_frame_id = 0;
  for (const auto &n: surfel_graph->nodes()) {
    for (const auto &fd: n->data()->frame_data()) {
      if (fd.pixel_in_frame.frame > max_frame_id) {
        max_frame_id = fd.pixel_in_frame.frame;
      }
    }
  }
  return max_frame_id + 1;
}

std::ostream &operator<<(std::ostream &output, const std::shared_ptr<Surfel> &surfel) {
  output << surfel->id();
  return output;
}

void
set_k(const SurfelGraphPtr &graph,
      const SurfelGraphNodePtr &node1, unsigned short k1,
      const SurfelGraphNodePtr &node2, unsigned short k2) {
  auto &edge = graph->edge(node1, node2);
  if (node1->data()->id() < node2->data()->id()) {
    edge->set_k_low(k1);
    edge->set_k_high(k2);
  } else {
    edge->set_k_low(k2);
    edge->set_k_high(k1);
  }
}

std::pair<unsigned short, unsigned short>
get_k(
    const SurfelGraphPtr &graph,
    const SurfelGraphNodePtr &node1,
    const SurfelGraphNodePtr &node2) {
  auto &edge = graph->edge(node1, node2);
  if (node1->data()->id() < node2->data()->id()) {
    return (std::make_pair(edge->k_low(), edge->k_high()));
  } else {
    return (std::make_pair(edge->k_high(), edge->k_low()));
  }
}

void
set_t(const SurfelGraphPtr &graph,
      const SurfelGraphNodePtr &node1, Eigen::Vector2i t1,
      const SurfelGraphNodePtr &node2, Eigen::Vector2i t2) {
  auto &edge = graph->edge(node1, node2);
  if (node1->data()->id() < node2->data()->id()) {
    edge->set_t_low(t1[0], t1[1]);
    edge->set_t_high(t2[0], t2[1]);
  } else {
    edge->set_t_low(t2[0], t2[1]);
    edge->set_t_high(t1[0], t1[1]);
  }
}

std::pair<Eigen::Vector2i, Eigen::Vector2i>
get_t(
    const SurfelGraphPtr &graph,
    const SurfelGraphNodePtr &node1,
    const SurfelGraphNodePtr &node2) {
  auto &edge = graph->edge(node1, node2);
  if (node1->data()->id() < node2->data()->id()) {
    return (std::make_pair(edge->t_low(), edge->t_high()));
  } else {
    return (std::make_pair(edge->t_high(), edge->t_low()));
  }
}
