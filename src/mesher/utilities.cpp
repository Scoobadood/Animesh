//
// Created by Dave Durbin on 28/11/19.
//

#include <vector>
#include <iostream>
#include <DepthMap/DepthMap.h>
#include <Properties/Properties.h>
#include "../mesher/depth_map_io.h"

std::vector<DepthMap>
load_depth_maps(const Properties &properties) {
    using namespace std;

    string source_directory = properties.getProperty("source-directory");
    float ts = properties.getFloatProperty("ts");
    float tl = properties.getFloatProperty("tl");
    vector<DepthMap> depth_maps = load_depth_maps(source_directory, ts, tl);
    return depth_maps;
}


/**
 * Load the cameras (one per frame)
 */
std::vector<Camera>
load_cameras(unsigned int num_frames) {
    std::vector<Camera> cameras;
    // TODO: Move to loading these from disk rather than constructing by hand.
    cameras.reserve(num_frames);
    for (unsigned int i = 0; i < num_frames; ++i) {
        cameras.emplace_back(
                (float[]) {0.0f, 0.0f, 5.0f}, // position
                (float[]) {0.0f, 0.0f, 0.0f}, // view
                (float[]) {0.0f, 1.0f, 0.0f}, // up
                (float[]) {640.0f, 480.0f},        // resolution
                (float[]) {35.0f, 35.0f},     // fov
                5.0f                 // focal distance
        );
    }
    return cameras;
}

/**
 * Take the vector of depth maps and subsample each, returnomng a new vector of the sumbsampled maps.
 * @param depth_maps The input vector
 * @return A new vector of submsampled maps.
 */
std::vector<DepthMap>
resample_depth_maps(const std::vector<DepthMap> &depth_maps) {
    using namespace std;

    vector<DepthMap> resampled_depth_maps;
    resampled_depth_maps.reserve(depth_maps.size());
    for (const auto &dm : depth_maps) {
        resampled_depth_maps.push_back(dm.resample());
    }
    return resampled_depth_maps;
}

/**
 * Construct depth map hierarch given a vector of sourcde depth maps
 */
std::vector<std::vector<DepthMap>>
create_depth_map_hierarchy(const Properties &properties, const std::vector<DepthMap> &depth_maps) {
    using namespace std;

    vector<vector<DepthMap>> depth_map_hierarchy;
    int num_levels = properties.getIntProperty("num-levels");
    cout << "Constructing depth map hierarchy with " << num_levels << " levels." << endl;
    depth_map_hierarchy.reserve(num_levels);
    depth_map_hierarchy.push_back(depth_maps);
    cout << "1 of " << num_levels << "    " << flush;
    for (int i = 2; i <= num_levels; i++) {
        cout << "\r" << i << " of " << num_levels << "    " << flush;
        depth_map_hierarchy.push_back(resample_depth_maps(depth_map_hierarchy.at(i-2)));
    }
    cout << endl;

    return depth_map_hierarchy;
}

