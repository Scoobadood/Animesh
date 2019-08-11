#include "surfel_compute.hpp"
#include "surfel_io.h"
#include "smooth.h"
#include "mesher_args.h"
#include "surfel_hierarchy.h"
#include <DepthMap/DepthMapPyramid.h>

//
// Launch with -s surfel_file or

//             -d <source_files_directory>
//             -c <correspondence_file_name> or if missing they will be assumed to be in source files directory.
//
int main( int argc, char *argv[] ) {
  using namespace std;

  MesherArguments args;
  parse_args( argc, argv, args);

  // Construct a hierarchy of surfels
  SurfelHierarchy sh(0.01f);

  sh.optimise();

  return 0;
}

