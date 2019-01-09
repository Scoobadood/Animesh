#include "perfTool.h"
#include "FileUtils/ObjFileParser.h"
#include "Field/FieldOptimiser.h"
#include "FileUtils/FileUtils.h"

#include <vector>
#include <iomanip>

const bool FACE_WISE = false;

namespace animesh{

std::vector<std::string> get_files_in_directory( std::string directory_name ) {
    using namespace std;

    vector<string> file_names;
    files_in_directory( directory_name, file_names, []( string name ) {
        using namespace std;

        std::transform(name.begin(), name.end(), name.begin(), ::tolower);
        string obj = ".obj";

        if( name.size() < 4 ) return false;
        return equal(obj.rbegin(), obj.rend(), name.rbegin());
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


/**
 * Load multiple files
 */
FieldOptimiser *
load_multiple_files( const std::vector<std::string>& file_names ) {
    using namespace std;

    // Sanity check
    assert( file_names.size() > 0 );
    for( string file_name : file_names ) {
        bool is_directory = false;
        assert (file_exists( file_name, is_directory ) && !is_directory );
    }

    // Sort files alphanumerically
    vector<string> sorted_file_names = file_names;
    sort(sorted_file_names.begin(), sorted_file_names.end());

    ObjFileParser parser;

    vector<vector<PointNormal::Ptr>>    frames;
    multimap<size_t, size_t>            adjacency;
    pair<vector<PointNormal::Ptr>, multimap<size_t, size_t>> results = parser.parse_file( sorted_file_names[0], true, FACE_WISE );
    frames.push_back( results.first );
    adjacency = results.second;

    for( size_t file_idx = 1; file_idx < sorted_file_names.size(); ++file_idx ) {
        vector<PointNormal::Ptr> frame_data = parser.parse_file( sorted_file_names[file_idx], false, FACE_WISE ).first;
        frames.push_back(frame_data);
    }
    return new FieldOptimiser(frames, adjacency);
}

/**
 * Load from directory.
 */
FieldOptimiser *
load_from_file( const std::string& file_name ) {
    using namespace std;

    ObjFileParser parser;
    vector<vector<PointNormal::Ptr>>    frames;
    multimap<size_t, size_t>            adjacency;
    pair<vector<PointNormal::Ptr>, multimap<size_t, size_t>> results = parser.parse_file( file_name, true);
    frames.push_back( results.first );
    adjacency = results.second;
    return new FieldOptimiser(frames, adjacency);
}

/**
 * Load from directory.
 */
FieldOptimiser *
load_from_directory( const std::string& dir_name ) {
    using namespace std;

    vector<string> file_names = get_files_in_directory( dir_name );
    return load_multiple_files( file_names );
}


/**
 * Load from one or more files or directory.
 * If file_names has more than one entry, they must all be files and we load them all
 * If it has one entry, it could be a directory or a file.
 */
FieldOptimiser *
load_from( const std::vector<std::string>& file_names ) {
    using namespace std;
    // Nothing
    if( file_names.size() == 0) return nullptr;

    // Either a single file or directory
    if( file_names.size() == 1 ) {
        string file_name = file_names[0];
        bool is_directory  =false;
        if( file_exists( file_name, is_directory ) ) {
            if( is_directory ) {
                return load_from_directory( file_name );
            } else {
                // Single file to open
                return load_from_file( file_name );
            }
        }
        // else file didn't exist
        return nullptr;
    } else {
        // Load all files in list
        return load_multiple_files(file_names);
    }
}
}

void timeObj( const std::string& file_name ) {
  using namespace std;
  using namespace animesh;

  using Clock = std::chrono::high_resolution_clock;
  auto start_time = Clock::now();

  FieldOptimiser * optimiser = load_from( vector<string>{file_name});

  auto load_time = Clock::now();
  int load_time_ms = chrono::duration_cast<chrono::milliseconds>(load_time - start_time).count();

  optimiser->optimise( );
  auto end_time = Clock::now();
  int optimise_time_ms = chrono::duration_cast<chrono::milliseconds>(end_time - load_time).count();

  int total_time_ms = chrono::duration_cast<chrono::milliseconds>(end_time - start_time).count();

  // Dump in tabular format
  cout << "|" << setw(19) << get_file_name_and_extension(file_name).first << " |"
        << setw(7) << optimiser->num_frames() << " |"
        << setw(7) << optimiser->num_tiers() << " |"
        << setw(7) << optimiser->num_nodes_in_tier(0) << " |"
        << setw(7) << optimiser->num_edges_in_tier(0) << " |"
        << setw(11) << load_time_ms << " |"
        << setw(11) << optimise_time_ms << " |"
        << setw(11) << total_time_ms <<  " |" << endl;
  cout << "+--------------------+--------+--------+--------+--------+------------+------------+------------+" << endl;
}

int main( int argc, char *argv[] ) {
  using namespace std;

  if( argc < 2 ) return 1;

  cout << "+--------------------+--------+--------+--------+--------+--------------------------------------+" << endl;
  cout << "|                    |        |        |        |        | Times (ms)                           |" << endl;
  cout << "|      Test          | Frames | Tiers  |  Vert  | Edges  |------------+------------+------------+" << endl;
  cout << "|                    |        |        |        |        |    Load    |  Optimise  |   Total    |" << endl;
  cout << "+--------------------+--------+--------+--------+--------+------------+------------+------------+" << endl;

  for( int i=1; i<argc; ++i) {
    timeObj(argv[i]);
  }

  return 0;
}
