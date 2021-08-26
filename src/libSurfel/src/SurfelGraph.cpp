//
// Created by Dave Durbin (Old) on 4/7/21.
//
#include "SurfelGraph.h"

std::vector <SurfelGraphNodePtr>
get_node_neighbours_in_frame(
        const SurfelGraphPtr &graph,
        const SurfelGraphNodePtr &node_ptr,
        unsigned int frame_index) {
    std::vector <SurfelGraphNodePtr> neighbours_in_frame;
    for (const auto &neighbour_node : graph->neighbours(node_ptr)) {
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
    for (const auto &n : surfel_graph->nodes()) {
        for (const auto &fd : n->data()->frame_data()) {
            if (fd.pixel_in_frame.frame > max_frame_id) {
                max_frame_id = fd.pixel_in_frame.frame;
            }
        }
    }
    return max_frame_id + 1;
}

std::ostream& operator<<( std::ostream& output, const std::shared_ptr<Surfel>& surfel ){
  output << surfel->id();
  return output;
}
