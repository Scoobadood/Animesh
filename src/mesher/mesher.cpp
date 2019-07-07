#include "surfel_compute.hpp"
#include "surfel_io.h"
#include "smooth.h"
#include "mesher_args.h"

//
// Launch with -s surfel_file or

//             -d <source_files_directory>
//             -c <correspondence_file_name> or if missing they will be assumed to be in source files directory.
//
int main( int argc, char *argv[] ) {
  using namespace std;

  MesherArguments args;
  parse_args( argc, argv, args);

  vector<Surfel> surfels;
  if( args.source == MesherArguments::FILE) {
    load_from_file(args.file_or_directory, surfels);
  } else { /* compute */
      compute_surfels(args, surfels);
    save_to_file( "surfel_table.bin", surfels);
  }
  // Now start smoothing
  optimise(surfels);
  save_to_file( "surfel_table_converged.bin", surfels);
  
  return 0;
}

