#include "surfel.hpp"
#include "smooth.h"
#include "args.h"

//
// Launch with -s surfel_file or 
//             -d source_files_directory
//
int main( int argc, char *argv[] ) {
  using namespace std;

  Arguments args;
  parse_args( argc, argv, args);

  vector<Surfel> surfels;
  if( args.load_source == Arguments::FILE) {
    load_from_file(args.file_or_directory, surfels);
  } else { /* use_directory */
    load_from_directory(args.file_or_directory, surfels);
    save_to_file( "surfel_table.bin", surfels);
  }
  // Now start smoothing
  optimise(surfels);
  save_to_file( "surfel_table_converged.bin", surfels);
  
  return 0;
}

