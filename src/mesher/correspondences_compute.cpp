#include <iostream>
#include <string>
#include <regex>
#include <map>

#include <FileUtils/PgmFileParser.h>

#include "surfel_compute.h"
#include "correspondences_compute.h"

static const char *VERTEX_FILE_NAME_REGEX = R"(\/{0,1}(?:[^\/]*\/)*vertex_[0-9]+\.pgm)";


/**
 * For the given directory, find all files which match the vertex REGEX above and add them to the vector.
 * Return the vector by name.
 * @param directory_name The directory from which to laod.
 * @return The vector of file names.
 */
std::vector<std::string>
get_vertex_files_in_directory(const std::string &directory_name) {
    using namespace std;

    const regex file_name_regex(VERTEX_FILE_NAME_REGEX);
    vector<string> file_names;
    files_in_directory(directory_name, file_names, [file_name_regex](string name) {
        std::transform(name.begin(), name.end(), name.begin(), ::tolower);
        return regex_match(name, file_name_regex);
    });

    // Construct full path names
    vector<string> full_path_names;
    for (auto const &file_name : file_names) {
        full_path_names.push_back(file_in_directory(directory_name, file_name));
    }
    sort(full_path_names.begin(), full_path_names.end());
    return full_path_names;
}

/**
 * Compute the correspondences between pixels in cleaned depth maps
 * Returns a vector of correspondences where each correspondence
 * is a PixelInFrame
 */
void
compute_correspondences(const std::string &source_directory,
                        std::vector<std::vector<PixelInFrame>> &correspondences) {
    using namespace std;
    using namespace Eigen;

    cout << "Computing correspondences..." << flush;

    auto file_names = get_vertex_files_in_directory(source_directory);

    // For each frame, load each pixel and for each pixel with non-zero vertex
    // add to a map.
    multimap<unsigned int, PixelInFrame> vertex_to_frame_pixel;
    size_t current_frame_idx = 0;
    for (const auto &file_name : file_names) {
        PgmData pgm = read_pgm(file_name);

        size_t source_pixel_idx = 0;
        for (std::size_t row = 0; row < pgm.height; ++row) {
            for (std::size_t col = 0; col < pgm.width; ++col) {
                int vertex = pgm.data.at(source_pixel_idx);
                // Ignore background
                if (vertex != 0 ) {
                    vertex_to_frame_pixel.insert(make_pair(vertex, PixelInFrame(col, row, current_frame_idx)));
                }
                ++source_pixel_idx;
            }
        }
        current_frame_idx++;
    }

    // We now have a map from vertices to all corresponding frame/pixel pairs
    // A correspondence is a vector of all frame/pixel pairs that have the same vertex
    correspondences.clear();
    vector<PixelInFrame> correspondence;
    for (auto it = vertex_to_frame_pixel.begin(); it != vertex_to_frame_pixel.end();) {
        correspondence.clear();
        unsigned int vertex_id = it->first;
        do {
            correspondence.push_back(it->second);
            ++it;
        } while ((it != vertex_to_frame_pixel.end()) && (vertex_id == it->first));
        correspondences.push_back(correspondence);
    }
    cout << endl;
}
