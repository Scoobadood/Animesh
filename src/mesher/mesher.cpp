#include <iostream>
#include <regex>
#include <FileUtils/FileUtils.h>
#include "surfel.hpp"
#include "pixel_correspondence.hpp"
#include "depth_image_loader.h"
#include "smooth.h"

std::vector<std::string> get_vertex_files_in_directory( std::string directory_name ) {
    using namespace std;

    vector<string> file_names;
    files_in_directory( directory_name, file_names, []( string name ) {
        using namespace std;

        std::transform(name.begin(), name.end(), name.begin(), ::tolower);

        const regex file_name_regex("\\/{0,1}(?:[^\\/]*\\/)*vertex_[0-9]+\\.pgm");
        return regex_match(name, file_name_regex);
    });
    // Construct full path names
    vector<string> full_path_names;
    for( string file_name : file_names) {
        // FIXME: Replace this evilness with something more robust and cross platform.
        string path_name = directory_name + "/" + file_name;
        full_path_names.push_back( path_name );
    }
    std::sort(full_path_names.begin(), full_path_names.end() );
    return full_path_names;
}

std::vector<std::string> get_depth_files_in_directory( std::string directory_name ) {
    using namespace std;

    vector<string> file_names;
    files_in_directory( directory_name, file_names, []( string name ) {
        using namespace std;

        std::transform(name.begin(), name.end(), name.begin(), ::tolower);

        const regex file_name_regex("\\/{0,1}(?:[^\\/]*\\/)*depth_[0-9]+\\.pgm");
        return regex_match(name, file_name_regex);
    });
    // Construct full path names
    vector<string> full_path_names;
    for( string file_name : file_names) {
        // FIXME: Replace this evilness with something more robust and cross platform.
        string path_name = directory_name + "/" + file_name;
        full_path_names.push_back( path_name );
    }
    std::sort(full_path_names.begin(), full_path_names.end() );
    return full_path_names;
}

std::vector<Surfel>
load_from_directory( const std::string& dir ) {
  using namespace std;

  cout << "Computing correspondences..." << flush;
  vector<string> files = get_vertex_files_in_directory(dir);
  vector<vector<pair<unsigned int, unsigned int>>> correspondences;
  compute_correspondences(files, correspondences);
  cout << " done." << endl;

  cout << "Loading depth images..." << flush;
  files = get_depth_files_in_directory(dir);
  vector<vector<vector<unsigned int>>> neighbours;
  vector<vector<PointWithNormal>> point_clouds;
  load_depth_images(files, point_clouds, neighbours);
  cout << " done." << endl;

  cout << "Building surfel table..." << flush;
  std::vector<Surfel> surfels = build_surfel_table(point_clouds, neighbours, correspondences);
  cout << " done." << endl;

  cout << "Saving..." << flush;
  save_to_file( surfels, point_clouds, "surfel_table.bin" );
  cout << " done." << endl;

  return surfels;
}

std::vector<Surfel>
load_from_surfel_file( const std::string& file_name) {
  using namespace std;

  cout << "Loading..." << flush;
  vector<Surfel> surfels;
  vector<vector<PointWithNormal>> point_normals;
  load_from_file(surfels, point_normals, file_name);
  cout << " done." << endl;
  return surfels;
}

void 
usage( const char *name ) {
    std::cout << "Usage : " << name << " [-s surfel_file | -d source_directory]" << std::endl;
    exit(-1);
}

//
// Launch with -s surfel_file or 
//             -d source_files_directory
//
int main( int argc, char *argv[] ) {
  using namespace std;

  vector<Surfel> surfels;
  bool args_ok = false;
  if( argc == 3 ) {
    if( argv[1][0] == '-') {
      if( argv[1][1] == 's' || argv[1][1] == 'S' ) {
        load_from_surfel_file(argv[2]);
        args_ok = true;
      } else if( argv[1][1] == 'd' || argv[1][1] == 'D' ) {
        load_from_directory(argv[2]);
        args_ok = true;
      }
    }
  }

  if( !args_ok )
    usage(argv[0]);

  return 0;
}
