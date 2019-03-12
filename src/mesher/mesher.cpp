#include <iostream>
#include <regex>
#include <FileUtils/FileUtils.h>
#include "surfel.hpp"
#include "pixel_correspondence.hpp"
#include "depth_image_loader.h"

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
    return full_path_names;
}

int main( int argc, char *argv[] ) {
  using namespace std;

  if( argc < 2 ) {
  	cerr << "Must specify source directory" << endl;
    exit(-1);
  }

  cout << "Computing correspondences...";
  string dir = argv[1];
  vector<string> files = get_vertex_files_in_directory(dir);
  vector<vector<pair<unsigned int, unsigned int>>> correspondences = compute_correspondences(files);
  cout << " done." << endl;

  cout << "Loading deth images...";
  files = get_depth_files_in_directory(dir);
  vector<vector<vector<unsigned int>>> neighbours;
  vector<vector<PointWithNormal>> point_clouds;
  load_depth_images(files, point_clouds, neighbours);
  cout << " done." << endl;

  cout << "Building surfel table...";
  std::vector<Surfel> surfels = build_surfel_table(point_clouds, neighbours, correspondences);
  cout << " done." << endl;

  cout << "Saving...";
  save_to_file( surfels, "surfel_table.bin" );
  cout << " done." << endl;

  return 0;
}
