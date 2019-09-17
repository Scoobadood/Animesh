#include <iostream>
#include <vector>
#include <Properties/Properties.h>
#include <DepthMap/DepthMap.h>
#include "depth_map_io.h"
#include "correspondences_compute.h"
#include "types.h"

void usage(const std::string &prog_name) {
    std::cout << "usage: " << prog_name << " [source_directory]" << std::endl;
    exit(-1);
}


std::vector<DepthMap>
resample_depth_maps(const std::vector<DepthMap>& depth_maps ) {
    using namespace std;

    vector<DepthMap> resampled_depth_maps;
    for( const auto& dm : depth_maps) {
        resampled_depth_maps.push_back(dm.resample());
    }
    return resampled_depth_maps;
}

int main(int argc, char *argv[]) {
    using namespace std;


    string property_file_name;
    if (argc == 2) {
        property_file_name = argv[1];
    } else {
        property_file_name = "animesh.properties";
    }

    Properties p{property_file_name};

    vector<vector<DepthMap>> depth_map_hierarchy;

    // Load depth maps
    string source_directory = p.getProperty("source-directory");
    float ts = p.getFloatProperty("ts");
    float tl = p.getFloatProperty("tl");
    vector<DepthMap> depth_maps = load_depth_maps(source_directory, ts, tl);
    depth_map_hierarchy.push_back(depth_maps);

    // Load cameras
    vector<Camera> cameras;
    // TODO: Move to loading these from disk rather than constructing by hand.
    for( int i=0; i<depth_maps.size(); ++i ) {
        cameras.emplace_back(
                (float[]){ 5.0f, 0.0f, 0.0f}, // position
                (float[]){ 0.0f, 0.0f, 0.0f}, // view
                (float[]){ 0.0f, 1.0f, 0.0f}, // up
                (float[]){ 40.0f, 30.0f },        // resolution
                (float[]){ 35.0f, 35.0f},     // fov
                5.0f                 // focal distance
                );
    }

    // Construct the hierarchy
    int num_levels = p.getIntProperty("num-levels");
    for ( int i=0; i<num_levels; i++ ) {
        depth_maps = resample_depth_maps(depth_maps);
        depth_map_hierarchy.push_back(depth_maps);
    }

    // For each level
    int level = num_levels - 1;

    while( level >= 0 ) {
        // Propagate chnages down

        // Generate correspondences
        vector<vector<PixelInFrame>> correspondences = compute_correspondences(cameras, depth_maps);

        // Generate Surfels for this level from correspondences

        // Smooth this level

    }

    return 0;
}
