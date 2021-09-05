//
// Created by Dave Durbin (Old) on 6/9/21.
//

#include "surfel_info.h"

#include <iostream>
#include "Surfel/Surfel_IO.h"
#include "Surfel/Surfel.h"

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

int main( int argc, const char * argv[] ) {
  if( argc != 2 ) {
    std::cout << "Usage: " << argv[0] << " <surfel_file_name>" << std::endl;
  }

  unsigned short flags;
  SurfelGraphPtr graph = load_surfel_graph_from_file(argv[1], flags);

  std::cout << "Surfels : " << graph->num_nodes() << std::endl;
  std::cout << "  Edges : " << graph->num_edges() << std::endl;
  std::cout << " Frames : " << get_num_frames(graph) << std::endl;
}