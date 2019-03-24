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

void
load_from_directory(  const std::string& dir, 
                      std::vector<Surfel>& surfels, 
                      std::vector<std::vector<PointWithNormal>>& point_clouds ) 
{
  using namespace std;

  cout << "Computing correspondences..." << flush;
  vector<string> files = get_vertex_files_in_directory(dir);
  vector<vector<pair<unsigned int, unsigned int>>> correspondences;
  compute_correspondences(files, correspondences);
  cout << " done." << endl;

  cout << "Loading depth images..." << flush;
  files = get_depth_files_in_directory(dir);
  vector<vector<vector<unsigned int>>> neighbours;
  load_depth_images(files, point_clouds, neighbours);
  cout << " done." << endl;

  cout << "Building surfel table..." << flush;
  surfels = build_surfel_table(point_clouds, neighbours, correspondences);
  cout << " done." << endl;

  cout << "Saving..." << flush;
  save_to_file( "surfel_table.bin", surfels, point_clouds);
  cout << " done." << endl;
}

void
load_from_surfel_file(  const std::string& file_name, 
                        std::vector<Surfel>& surfels, 
                        std::vector<std::vector<PointWithNormal>>& point_normals )
{
  using namespace std;

  cout << "Loading..." << flush;
  load_from_file(file_name, surfels, point_normals);
  cout << " done." << endl;
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
  vector<vector<PointWithNormal>> point_normals;
  bool args_ok = false;
  if( argc == 3 ) {
    if( argv[1][0] == '-') {
      if( argv[1][1] == 's' || argv[1][1] == 'S' ) {
        load_from_surfel_file(argv[2], surfels, point_normals);
        args_ok = true;
      } else if( argv[1][1] == 'd' || argv[1][1] == 'D' ) {
        load_from_directory(argv[2], surfels, point_normals);
        args_ok = true;
      }
    }
  }
  if( !args_ok )
    usage(argv[0]);

  return 0;
}
