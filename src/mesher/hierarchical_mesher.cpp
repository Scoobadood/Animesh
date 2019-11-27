#include <iostream>
#include <vector>
#include <memory>
#include <Properties/Properties.h>
#include <DepthMap/DepthMap.h>
#include <omp.h>
#include "correspondences_compute.h"
#include "correspondences_io.h"
#include "surfel_compute.h"
#include "optimise.h"
#include "types.h"
#include "surfel_io.h"
#include "utilities.h"

const char pre_smooth_filename_template[] = "/Users/dave/Desktop/presmooth_%02d.bin";
const char post_smooth_filename_template[] = "/Users/dave/Desktop/smoothed_%02d.bin";

/**
 * Given a set of existing surfels (previous level), populate the current level
 * by propagating the existing level surfel to all 'parent' surfels of the next level.
 * <pre>
   For each Surfel s in previous_level
     For each pixel in frame pif for s
       Derive the candidate source pixels in surfels, pif_next[] (there will be at most four)
       Store reference from pif_next[] to tangent of s

    For each surfel s in surfels
      For each pixel in frame pif for s
        Look up tangent(s) for pif and average them to bootstrap tangent for s

 * @param surfels Vector of surfels to be initialised
 * @param previous_level Vector of surfels from which to initialise
 */
void
initialise_surfel_tangents(std::vector<Surfel> &surfels, const std::vector<Surfel> &previous_level) {
    using namespace std;
    using namespace Eigen;

    cout << "Initialising surfels from pervious level" << endl;

    if (previous_level.empty())
        throw runtime_error("Unexpectedly found previous_level was empty");

    // TODO: Bootstrap orientation vectors from the previous level
    map<PixelInFrame, Vector3f> pif_to_tangent;
    for (const auto &surfel : previous_level) {
        for (const auto &frame : surfel.frame_data) {
            pif_to_tangent.emplace(
                    PixelInFrame{frame.pixel_in_frame.pixel.x * 2, frame.pixel_in_frame.pixel.y * 2,
                                 frame.pixel_in_frame.frame}, surfel.tangent);
            pif_to_tangent.emplace(
                    PixelInFrame{frame.pixel_in_frame.pixel.x * 2, frame.pixel_in_frame.pixel.y * 2 + 1,
                                 frame.pixel_in_frame.frame}, surfel.tangent);
            pif_to_tangent.emplace(
                    PixelInFrame{frame.pixel_in_frame.pixel.x * 2 + 1, frame.pixel_in_frame.pixel.y * 2,
                                 frame.pixel_in_frame.frame}, surfel.tangent);
            pif_to_tangent.emplace(
                    PixelInFrame{frame.pixel_in_frame.pixel.x * 2 + 1, frame.pixel_in_frame.pixel.y * 2 + 1,
                                 frame.pixel_in_frame.frame}, surfel.tangent);
        }
    }

    for (auto &surfel : surfels) {
        int count = 0;
        Vector3f mean_tangent{0.0f, 0.0f, 0.0};
        for (const auto &frame : surfel.frame_data) {
            auto it = pif_to_tangent.find(frame.pixel_in_frame);
            if (it != pif_to_tangent.end()) {
                mean_tangent += it->second;
                ++count;
            } else {
                cout << "Warning. Could not find entry for PIF (" << frame.pixel_in_frame.pixel.x << ", "
                     << frame.pixel_in_frame.pixel.y
                     << ", " << frame.pixel_in_frame.frame << ") in initialiser table." << endl;
            }
        }
        if (count != 0) {
            surfel.tangent = (mean_tangent / count).normalized();
        } else {
            throw runtime_error("Pixel in layer does not have any corresponding pixels in the next layer.");
        }
    }
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

std::vector<std::vector<PixelInFrame>>
get_correspondences(const Properties &properties,
                    unsigned int level,
                    const std::vector<std::vector<DepthMap>> &depth_map_hierarchy,
                    std::vector<Camera>& cameras) {
    using namespace std;

    vector<vector<PixelInFrame>> correspondences;

    if (properties.getBooleanProperty("load-correspondences")) {
        string corr_file_template = properties.getProperty("correspondence-file-template");
        size_t size = snprintf(nullptr, 0, corr_file_template.c_str(), level) + 1; // Extra space for '\0'
        std::unique_ptr<char[]> buf(new char[size]);
        snprintf(buf.get(), size, corr_file_template.c_str(), level);
        string file_name = std::string(buf.get(), buf.get() + size - 1); // We don't want the '\0' inside

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
    vector<Camera> cameras = load_cameras(num_frames);

    // Construct the hierarchy: number of levels as specified in properties.
    vector<vector<DepthMap>> depth_map_hierarchy = create_depth_map_hierarchy(properties, depth_maps);
    int num_levels = depth_map_hierarchy.size();
    // Compute normals
    for (unsigned int l = 0; l < num_levels; ++l) {
        for (unsigned int f = 0; f < num_frames; ++f) {
            depth_map_hierarchy.at(l).at(f).compute_normals(cameras.at(f));
        }
    }

    // For each level
    int level = num_levels - 1;
    vector<Surfel> previous_level;
    size_t surfels_per_step = properties.getIntProperty("surfels-per-step");

    while (level >= 0) {
        cout << "Level : " << level << endl;

        // Generate or load correspondences
        // TODO: Seed correspondences for next level Propagate changes down
        vector<vector<PixelInFrame>> correspondences = get_correspondences(properties, level, depth_map_hierarchy,
                                                                                 cameras);

        // Generate Surfels for this level from correspondences
        vector<Surfel> surfels = generate_surfels(depth_map_hierarchy.at(level), correspondences);

        cout << " Initialising tangents" << endl;

        // Populate with values from previous level if they exist
        if( previous_level.size() > 0 ) {
            initialise_surfel_tangents(surfels, previous_level);
        }

        // Save the pre-smoothing surfels in a renderable way
        char out_file_name[strlen(pre_smooth_filename_template) + 1];
        sprintf(out_file_name, pre_smooth_filename_template, level);
        save_to_file(out_file_name, surfels);

        // Smooth this level
        Optimiser o{0.1, num_frames, surfels_per_step};
        o.optimise(surfels);

        // Save the smoothed surfels in a renderable way
        char out_file_name2[strlen(post_smooth_filename_template) + 1];
        sprintf(out_file_name2, post_smooth_filename_template, level);
        save_to_file(out_file_name2, surfels);

        // Copy surfels into previous level for propagation down.
        previous_level = surfels;

        --level;
    }


    return 0;
}
