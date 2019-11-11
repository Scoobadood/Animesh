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
#include "surfel_io.h"

/**
 * Print usage instructions if the number of arguments is wrong or arguments are inconsistent
 *
 * @param prog_name The name of the program with path.
 */
void usage(const std::string &prog_name) {
    std::cout << "usage: " << prog_name << " [source_directory]" << std::endl;
    exit(-1);
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
    for (const auto &dm : depth_maps) {
        resampled_depth_maps.push_back(dm.resample());
    }
    return resampled_depth_maps;
}

void
initialise_surfels(std::vector<Surfel> &surfels, const std::vector<Surfel> &previous_level) {
    using namespace std;
    using namespace Eigen;

    if (previous_level.empty()) return;

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
                    PixelInFrame{fd.pixel_in_frame.pixel.x * 2, fd.pixel_in_frame.pixel.y * 2, fd.pixel_in_frame.frame},
                    surfel.tangent);
            pif_to_tangent.emplace(
                    PixelInFrame{fd.pixel_in_frame.pixel.x * 2, fd.pixel_in_frame.pixel.y * 2 + 1,
                                 fd.pixel_in_frame.frame},
                    surfel.tangent);
            pif_to_tangent.emplace(
                    PixelInFrame{fd.pixel_in_frame.pixel.x * 2 + 1, fd.pixel_in_frame.pixel.y * 2,
                                 fd.pixel_in_frame.frame},
                    surfel.tangent);
            pif_to_tangent.emplace(
                    PixelInFrame{fd.pixel_in_frame.pixel.x * 2 + 1, fd.pixel_in_frame.pixel.y * 2 + 1,
                                 fd.pixel_in_frame.frame},
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
                cout << "Warning. Could not find entry for PIF (" << fd.pixel_in_frame.pixel.x << ", "
                     << fd.pixel_in_frame.pixel.y
                     << ", " << fd.pixel_in_frame.frame << ") in initialiser table." << endl;
            }
            if (count != 0) {
                surfel.tangent = (tangent / count).normalized();
            }
        }
    }
}


/**
 * Load the cameras (one per frame)
 */
std::vector<const Camera>
load_cameras(unsigned int num_frames) {
    std::vector<const Camera> cameras;
    // TODO: Move to loading these from disk rather than constructing by hand.
    cameras.reserve(num_frames);
    for (unsigned int i = 0; i < num_frames; ++i) {
        cameras.emplace_back(
                (float[]) {5.0f, 0.0f, 0.0f}, // position
                (float[]) {0.0f, 0.0f, 0.0f}, // view
                (float[]) {0.0f, 1.0f, 0.0f}, // up
                (float[]) {40.0f, 30.0f},        // resolution
                (float[]) {35.0f, 35.0f},     // fov
                5.0f                 // focal distance
        );
    }
    return cameras;
}

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
 * Construct depth map hierarch given a vector of sourcde depth maps
 */
std::vector<std::vector<DepthMap>>
create_depth_map_hierarchy(const Properties &properties, std::vector<DepthMap> &depth_maps) {
    using namespace std;

    vector<vector<DepthMap>> depth_map_hierarchy;
    int num_levels = properties.getIntProperty("num-levels");
    cout << "Constructing depth map hierarchy with " << num_levels << " levels." << endl;
    depth_map_hierarchy.reserve(num_levels);
    depth_map_hierarchy.push_back(depth_maps);
    cout << "1 of " << num_levels << "    " << flush;
    for (int i = 2; i <= num_levels; i++) {
        cout << "\r" << i << " of " << num_levels << "    " << flush;
        depth_map_hierarchy.push_back(resample_depth_maps(depth_maps));
    }
    cout << endl;

    return depth_map_hierarchy;
}

std::vector<std::vector<const PixelInFrame>>
get_correspondences(const Properties &properties,
                    unsigned int level,
                    const std::vector<std::vector<DepthMap>> &depth_map_hierarchy,
                    const std::vector<const Camera> &cameras) {
    using namespace std;

    vector<vector<const PixelInFrame>> correspondences;

    if (properties.getBooleanProperty("load-correspondences")) {
        string corr_file_template = properties.getProperty("correspondence-file-template");
        size_t size = snprintf(nullptr, 0, corr_file_template.c_str(), level) + 1; // Extra space for '\0'
        std::unique_ptr<char[]> buf(new char[size]);
        snprintf(buf.get(), size, corr_file_template.c_str(), level);
        string file_name = std::string(buf.get(), buf.get() + size - 1); // We don't want the '\0' inside

        cout << "Loading correspondences from file " << file_name << endl;
        load_correspondences_from_file(file_name, correspondences);
    } else {
        cout << "Computing correspondences from scratch" << endl;
        correspondences = compute_correspondences(cameras, depth_map_hierarchy.at(level));
    }

    if (correspondences.empty()) {
        throw runtime_error("No correspondences found");
    }
    return correspondences;
}

/**
 * Entry point
 * Generate a vector of Surfels and save to disk.
 *
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, char *argv[]) {
    using namespace std;

    string property_file_name = (argc == 2) ? argv[1] : "animesh.properties";
    Properties properties{property_file_name};

    // Load depth maps
    vector<DepthMap> depth_maps = load_depth_maps(properties);
    size_t num_frames = depth_maps.size();

    // Load cameras
    vector<const Camera> cameras = load_cameras(num_frames);

    // Construct the hierarchy
    vector<vector<DepthMap>> depth_map_hierarchy = create_depth_map_hierarchy(properties, depth_maps);
    int num_levels = depth_map_hierarchy.at(0).size();

    // For each level
    int level = num_levels - 1;
    vector<Surfel> surfels;
    vector<Surfel> previous_level;
    size_t surfels_per_step = properties.getIntProperty("surfels-per-step");

    while (level > 0) {
        cout << "Level : " << level << endl;

        // Generate or load correspondences
        // TODO: Seed correspondences for next level Propagate changes down
        vector<vector<const PixelInFrame>> correspondences = get_correspondences(properties, level, depth_map_hierarchy,
                                                                                 cameras);

        // Generate Surfels for this level from correspondences
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

    // Save the smoothed surfels in a renderable way
    save_to_file("/Users/dave/Desktop/surfel.bin", surfels);

    return 0;
}
