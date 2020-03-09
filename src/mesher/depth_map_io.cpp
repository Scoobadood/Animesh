//
// Created by Dave Durbin on 2019-08-21.
//

#include <vector>
#include <regex>
#include <iostream>
#include <fstream>
#include <FileUtils/PgmFileParser.h>
#include <DepthMap/DepthMap.h>
#include <DepthMap/Normals.h>

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

void
save_depth_map_as_pgm(const std::string& file_name, const DepthMap& depth_map) {
    using namespace std;

    ofstream file{file_name};

    // Find min/max
    float min_depth = depth_map.depth_at(0, 0);
    float max_depth = depth_map.depth_at(0, 0);

    for( unsigned int row = 0; row < depth_map.height(); ++row) {
        for(unsigned int col = 0; col < depth_map.width(); ++col ) {
            float d = depth_map.depth_at(col,row);
            if (d < min_depth) min_depth = d;
            if (d > max_depth) max_depth = d;
        }
    }

    float range = max_depth - min_depth;

    file <<  "P2" << endl << depth_map.width() << " " << depth_map.height() << endl << "255" << endl;
    for( unsigned int row = 0; row < depth_map.height(); ++row) {
        for(unsigned int col = 0; col < depth_map.width(); ++col ) {
            float d = (depth_map.depth_at(col, row) - min_depth) / range;
            file << (int)(round(d * 255)) << " ";
        }
        file << endl;
    }
}

std::vector<int>
normal_to_colour( const NormalWithType& nwt ) {
    std::vector<int> rgb;
    if( nwt.type == NONE) {
        rgb.push_back(0);
        rgb.push_back(0);
        rgb.push_back(0);
    } else {
        rgb.push_back(round((nwt.x + 1.0f) * 255 * 0.5));
        rgb.push_back(round((nwt.y + 1.0f) * 255 * 0.5));
        rgb.push_back(round((nwt.z + 1.0f) * 255 * 0.5));
    }
    return rgb;
}

void
save_normals_as_ppm(const std::string& file_name, const DepthMap& depth_map) {
    using namespace std;

    ofstream file{file_name};
    file <<  "P3" << endl << depth_map.width() << " " << depth_map.height() << endl << "255" << endl;
    for( unsigned int y = 0; y < depth_map.height(); ++y) {
        for(unsigned int x = 0; x < depth_map.width(); ++x ) {
            auto n = depth_map.normal_at(x, y);
            auto nc = normal_to_colour(n);
            file << (int) (round(nc.at(0))) << " " << (int) (round(nc.at(1))) << " " << (int) (round(nc.at(2)))
                 << "     ";
        }
        file << endl;
    }
}