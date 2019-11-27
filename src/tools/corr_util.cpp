//
// Created by Dave Durbin on 5/10/19.
//

#include <vector>
#include <fstream>
#include <string>
#include "DepthMap/DepthMap.h"
#include "Properties/Properties.h"
#include "Camera/Camera.h"
#include "../mesher/depth_map_io.h"
#include "../mesher/types.h"
#include "../mesher/io_utils.h"
#include "../mesher/correspondences_compute.h"
#include "../mesher/correspondences_io.h"
#include "../mesher/utilities.h"


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
    size_t num_frames = depth_maps.size();

    size_t surfels_per_step = p.getIntProperty("surfels-per-step");

    // Load cameras
    // TODO: Move to loading these from disk rather than constructing by hand.
    vector<Camera> cameras;
    for (int i = 0; i < depth_maps.size(); ++i) {
        cameras.emplace_back(
                (float[]) {5.0f, 0.0f, 0.0f}, // position
                (float[]) {0.0f, 0.0f, 0.0f}, // view
                (float[]) {0.0f, 1.0f, 0.0f}, // up
                (float[]) {40.0f, 30.0f},        // resolution
                (float[]) {35.0f, 35.0f},     // fov
                5.0f                 // focal distance
        );
    }

    // Construct the hierarchy
    depth_map_hierarchy = create_depth_map_hierarchy(p, depth_maps);
    int num_levels = depth_maps.size();

    // For each level
    int level = num_levels - 1;
    vector<Surfel> previous_level;
    while (level >= 0) {
        cout << "Saving level " << level << "correspondences" << endl;
        // Generate file name for the level
        // Write trhe data using surfel_io methods
        // run this baby.
        char file_name[1024];
        sprintf(file_name, "level_%02d_corr.bin", level);
        ofstream file{file_name, ios::out | ios::binary};
        // Generate correspondences
        vector<vector<PixelInFrame>> correspondences = compute_correspondences(cameras, depth_map_hierarchy.at(level));
        save_correspondences_to_file(file_name,correspondences);

        --level;
    }

    return 0;
}
