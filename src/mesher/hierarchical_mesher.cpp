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

int main(int argc, char *argv[]) {
    using namespace std;


    string property_file_name;
    if (argc == 2) {
        property_file_name = argv[1];
    } else {
        property_file_name = "animesh.properties";
    }

    Properties p{property_file_name};


    // Load depth maps
    string source_directory = p.getProperty("source-directory");
    float ts = p.getFloatProperty("ts");
    float tl = p.getFloatProperty("tl");
    vector<DepthMap> depth_maps = load_depth_maps(source_directory, ts, tl);

    // generate correspondences
    vector<vector<PixelInFrame>> correspondences = compute_correspondences(depth_maps);



    return 0;
}
