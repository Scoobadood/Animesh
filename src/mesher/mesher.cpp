#include <iostream>
#include <string>
#include "surfel.hpp"
#include "smooth.h"

// TODO: Remove me once we finish with matlab
#include "depth_image_loader.h"

void 
usage( const char *name ) {
    std::cout << "Usage : " << name << " [-s surfel_file | -d source_directory] [-m]" << std::endl;
    exit(-1);
}

void
mat_dumper(const std::string& outfile, 
            int frame,
            const std::vector<Surfel>& surfels, 
            const std::vector<std::vector<PointWithNormal>>& point_normals ) {
  using namespace std;
  using namespace Eigen;


  cout << "Writing matlab file  " << outfile << "..." << flush;

  ofstream fout( outfile );
  for ( auto surfel : surfels ) {
    for( auto fd : surfel.frame_data) {
      if( fd.frame_idx == frame ) {
        Vector3f point1 = point_normals.at(frame).at(fd.point_idx).point;
        Vector3f n = point_normals.at(frame).at(fd.point_idx).normal;
        n.normalize();
        Vector3f t = fd.transform * surfel.tangent;
        t.normalize();

        Vector3f point2 = point1 + n;
        // Vector3f point3 = point1 + ttan;
        // Vector3f point4 = point1 - ttan;

        // fout << point3.x() << "  " << point3.y() << "  " << point3.z() << "  " << endl;
        // fout << point4.x() << "  " << point4.y() << "  " << point4.z() << "  " << endl;
        // fout << "NaN NaN NaN" << endl;
        fout << point1.x() << "  " << point1.y() << "  " << point1.z() << "  " << endl;
        fout << point2.x() << "  " << point2.y() << "  " << point2.z() << "  " << endl;
        fout << "NaN NaN NaN" << endl;
      }
    }
  }
  cout << " done." << endl;
}

bool 
handle_args(int argc, char *argv[], 
            std::string& file_or_directory, 
            bool& load_from_file, 
            bool& load_from_directory,
            bool& dump
            ) {
  bool args_ok = false;
  if( argc >= 3 ) {
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
  if( argc > 3 ) {
    args_ok = false;
    if(( argv[3][0] == '-') && (argv[3][1] == 'm' || argv[3][1] == 'M')) {
      args_ok = true;
      dump = true;
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
  bool dump = false;
  bool args_ok = handle_args( argc, argv, file_or_directory, use_file, use_directory, dump);

  if( !args_ok ) exit(-1);

  if( use_file) {
    cout << "Loading from file " << file_or_directory << "..." << flush;
    load_from_file(file_or_directory, surfels, point_normals);
    cout << " done." << endl;
  } else { /* use_directory */
    cout << "Loading from directory " << file_or_directory << "..." << flush;
    load_from_directory(file_or_directory, surfels, point_normals);
  }

  if( dump ) {
    mat_dumper("mat.txt", 2, surfels, point_normals);  
  } else {

    // Now start smoothing
    optimise(surfels, point_normals);

    save_to_file( "surfel_table_converged.bin", surfels, point_normals);
  }
  return 0;
}

