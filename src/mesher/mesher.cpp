#include <iostream>
#include <string>
#include "surfel.hpp"
#include "smooth.h"


void 
usage( const char *name ) {
    std::cout << "Usage : " << name << " [-s surfel_file | -d source_directory]" << std::endl;
    exit(-1);
}

bool 
handle_args(int argc, char *argv[], 
            std::string& file_or_directory, 
            bool& load_from_file, 
            bool& load_from_directory) {
  bool args_ok = false;
  if( argc == 3 ) {
    file_or_directory = argv[2];
    if( argv[1][0] == '-') {
      if( argv[1][1] == 's' || argv[1][1] == 'S' ) {
          args_ok = true;
          load_from_file = true;
          load_from_directory = false;
      } else if( argv[1][1] == 'd' || argv[1][1] == 'D' ) {
          args_ok = true;
          load_from_file = false;
          load_from_directory = true;
      }
    }
  }
  if( !args_ok )
    usage(argv[0]);
  return args_ok;
}

//
// Launch with -s surfel_file or 
//             -d source_files_directory
//
int main( int argc, char *argv[] ) {
  using namespace std;

  vector<Surfel> surfels;
  vector<vector<PointWithNormal>> point_normals;
  string file_or_directory;
  bool use_file = false;
  bool use_directory = false;
  bool args_ok = handle_args( argc, argv, file_or_directory, use_file, use_directory);

  if( !args_ok ) exit(-1);

  if( use_file) {
    cout << "Loading from file " << file_or_directory << "..." << flush;
    load_from_file(file_or_directory, surfels, point_normals);
    cout << " done." << endl;
  } else { /* use_directory */
    cout << "Loading from directory " << file_or_directory << "..." << flush;
    load_from_directory(file_or_directory, surfels, point_normals);
  }

  // Now start smoothing
  optimise(surfels, point_normals);

  save_to_file( "surfel_table_converged.bin", surfels, point_normals);

  return 0;
}
