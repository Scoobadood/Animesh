#include <iostream>
#include <vector>
#include <memory>
#include <Properties/Properties.h>
#include <DepthMap/DepthMap.h>
#include <omp.h>
#include "depth_map_io.h"
#include "correspondences_compute.h"
#include "correspondences_io.h"
#include "surfel_compute.h"
#include "optimise.h"
#include "types.h"

void usage(const std::string &prog_name) {
    std::cout << "usage: " << prog_name << " [source_directory]" << std::endl;
    exit(-1);
}


std::vector<DepthMap>
resample_depth_maps(const std::vector<DepthMap> &depth_maps) {
    using namespace std;

    vector<DepthMap> resampled_depth_maps;
    for (const auto &dm : depth_maps) {
        resampled_depth_maps.push_back(dm.resample());
    }
    return resampled_depth_maps;
}

void
initialise_surfels(std::vector<Surfel> &surfels, const std::vector<Surfel> &previous_level) {
    using namespace std;
    using namespace Eigen;

    if( previous_level.empty()) return;

    // TODO: Bootstrap orientation vectors from the previous level
    /*
     For each Surfel s in level n
      For each pixel in frame pif for s
         Derive the source pixels in level n-1 pif_next (there will be at most four)
            Store reference from pif_next to tangent of s

    Generate surfels at n-1
    For each surfel s at n-1
      For each pixel in frame pif for s
        Look up tangent(s) for pif and average them to bootstrap tangent for s

     */
    map<PixelInFrame, Vector3f> pif_to_tangent;
    for (const auto &surfel : previous_level) {
        for (const auto &fd : surfel.frame_data) {
            pif_to_tangent.emplace(
                    PixelInFrame{fd.pixel_in_frame.x * 2, fd.pixel_in_frame.y * 2, fd.pixel_in_frame.frame},
                    surfel.tangent);
            pif_to_tangent.emplace(
                    PixelInFrame{fd.pixel_in_frame.x * 2, fd.pixel_in_frame.y * 2 + 1, fd.pixel_in_frame.frame},
                    surfel.tangent);
            pif_to_tangent.emplace(
                    PixelInFrame{fd.pixel_in_frame.x * 2 + 1, fd.pixel_in_frame.y * 2, fd.pixel_in_frame.frame},
                    surfel.tangent);
            pif_to_tangent.emplace(
                    PixelInFrame{fd.pixel_in_frame.x * 2 + 1, fd.pixel_in_frame.y * 2 + 1, fd.pixel_in_frame.frame},
                    surfel.tangent);
        }
    }

    Vector3f tangent{0.0f, 0.0f, 0.0};
    int count = 0;
    for (auto &surfel : surfels) {
        for (const auto &fd : surfel.frame_data) {
            auto it = pif_to_tangent.find(fd.pixel_in_frame);
            if (it != pif_to_tangent.end()) {
                tangent += it->second;
                ++count;
            } else {
                cout << "Warning. Could not find entry for PIF (" << fd.pixel_in_frame.x << ", " << fd.pixel_in_frame.y
                     << ", " << fd.pixel_in_frame.frame << ") in initialiser table." << endl;
            }
            if( count != 0 ) {
                surfel.tangent = (tangent / count).normalized();
            }
        }
    }
}


int main(int argc, char *argv[]) {
    using namespace std;

    string property_file_name =  (argc == 2) ? argv[1] : "animesh.properties";
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
    cout << "Constructing depth map hierarchy" << endl;
    int num_levels = p.getIntProperty("num-levels");
    for (int i = 0; i < num_levels; i++) {
        cout << "\r" << i << " of " << num_levels << "    " << flush;
        depth_maps = resample_depth_maps(depth_maps);
        depth_map_hierarchy.push_back(depth_maps);
    }
    cout << endl;

    // For each level
    int level = num_levels - 1;
    vector<Surfel> previous_level;
    while (level >= 0) {
        cout << "Level : " << level << endl;
        // TODO: Seed correspondences for next level Propagate changes down

        // Generate or load correspondences
        vector<vector<PixelInFrame>> correspondences;
        if( p.getBooleanProperty("load-correspondences") ) {
            string corr_file_template = p.getProperty("correspondence-file-template");
            size_t size = snprintf( nullptr, 0, corr_file_template.c_str(), level ) + 1; // Extra space for '\0'
            std::unique_ptr<char[]> buf( new char[ size ] );
            snprintf( buf.get(), size, corr_file_template.c_str(), level );
            string file_name = std::string( buf.get(), buf.get() + size - 1 ); // We don't want the '\0' inside
            // TODO: Make this compile
            cout << "Loading correspondences from file " << file_name << endl;
            load_correspondences_from_file(file_name, correspondences);
        } else {
            cout << "Computing correspondences from scratch" << endl;
            correspondences = compute_correspondences(cameras, depth_map_hierarchy.at(level));
        }


        // Generate Surfels for this level from correspondences
        vector<Surfel> surfels;
        generate_surfels(depth_map_hierarchy.at(level), correspondences, surfels);

        // Populate with values from previous level if they exist
        initialise_surfels(surfels, previous_level);

        // Smooth this level
        Optimiser o{0.1, num_frames, surfels_per_step};
        o.optimise(surfels);

        // Copy surfels into previous level for propagation down.
        previous_level = surfels;

        --level;
    }

    return 0;
}
