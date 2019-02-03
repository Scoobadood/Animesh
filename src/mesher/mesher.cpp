#include <iostream>
#include "surfel.hpp"
#include <regex>
#include <FileUtils/FileUtils.h>

std::vector<std::string> get_files_in_directory( std::string directory_name ) {
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
  }

  string dir = argv[1];
  vector<string> files = get_files_in_directory(dir);

  for( auto fn : files ) {
  	cout << fn << endl;
  }

  return 0;
}
