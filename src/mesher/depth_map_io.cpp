//
// Created by Dave Durbin on 2019-08-21.
//

#include <vector>
#include <regex>

#include <FileUtils/PgmFileParser.h>


#include "depth_map_io.h"

static const char *DEPTH_FILE_NAME_REGEX = R"(\/{0,1}(?:[^\/]*\/)*depth[0-9]+\.mat)";

std::vector<std::string>
get_depth_files_in_directory(const std::string &directory_name) {
    using namespace std;

    vector<string> file_names;
    files_in_directory(directory_name, file_names, [](string name) {
        using namespace std;

        std::transform(name.begin(), name.end(), name.begin(), ::tolower);

        const regex file_name_regex(DEPTH_FILE_NAME_REGEX);
        return regex_match(name, file_name_regex);
    });
    // Construct full path names
    vector<string> full_path_names;
    for (auto const &file_name : file_names) {
        string path_name = file_in_directory(directory_name, file_name);
        full_path_names.push_back(path_name);
    }
    std::sort(full_path_names.begin(), full_path_names.end());
    return full_path_names;
}


/**
 * Load the depth maps from a list of files into memory
 * as a vector.
 */
void
do_load(const std::string &source_directory, //
                        float ts, //
                        float tl, //
                        std::vector<DepthMapPyramid> &depth_map_pyramids) {
    using namespace std;

    vector<string> files = get_depth_files_in_directory(source_directory);
    if (files.empty()) {
        throw runtime_error("No depth images found in " + source_directory);
    }

    depth_map_pyramids.clear();
    int count = 0;
    int target = files.size();
    for (const auto &file_name : files) {
        cout << " \r" << ++count << " of " << target << flush;
        DepthMap dm{file_name};
        dm.cull_unreliable_depths(ts, tl);
        dm.get_normals();
        depth_map_pyramids.emplace_back(dm);
    }
    cout << endl << "Read " << depth_map_pyramids.size() << " files." << endl;
}

/**
 * Load the depth maps according to the provided arguments.
 * @param args The arguments specifying how to load.
 * @param pyramids The vector to be populated with DephgMapPyramids
 */
void
load_depth_map_pyramids(MesherArguments &args, std::vector<DepthMapPyramid> &pyramids) {
    const std::string dir = args.file_or_directory;
    std::cout << "Loading from directory " << dir << "..." << std::endl;

    do_load(dir, args.ts, args.tl, pyramids);
}

std::vector<DepthMap>
load_depth_maps(const std::string& source_directory, float ts, float tl) {
    using namespace std;

    cout << "Loading depth maps from " << source_directory << endl;

    vector<DepthMap> depth_maps;
    vector<string> depth_file_names = get_depth_files_in_directory(source_directory);
    if (depth_file_names.empty()) {
        throw runtime_error("No depth images found in " + source_directory);
    }

    int count = 0;
    int target = depth_file_names.size();
    for (const auto &file_name : depth_file_names) {
        cout << " \r  " << ++count << " of " << target << flush;
        depth_maps.emplace_back(file_name);
        depth_maps.back().cull_unreliable_depths(ts, tl);
    }
    cout << endl << "  done. " << endl;

    return depth_maps;
}
