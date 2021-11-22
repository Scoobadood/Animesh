//
// Created by Dave Durbin (Old) on 6/9/21.
//

#include "surfel_info.h"

#include <iostream>
#include "Surfel/Surfel_IO.h"
#include "Surfel/Surfel.h"


int main( int argc, const char * argv[] ) {
  if( argc != 2 ) {
    std::cout << "Usage: " << argv[0] << " <surfel_file_name>" << std::endl;
  }

  unsigned short flags;
  std::default_random_engine rng{123};         // the Mersenne Twister with a popular choice of parameters

  SurfelGraphPtr graph = load_surfel_graph_from_file(argv[1], flags, rng);

  std::cout << "Surfels : " << graph->num_nodes() << std::endl;
  std::cout << "  Edges : " << graph->num_edges() << std::endl;
  std::cout << " Frames : " << get_num_frames(graph) << std::endl;
}