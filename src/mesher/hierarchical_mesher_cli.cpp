#include <iostream>
#include <vector>
#include <map>
#include <memory>
#include <Properties/Properties.h>
#include <DepthMap/DepthMap.h>
#include <omp.h>
#include "correspondences_io.h"
#include "surfel_compute.h"
#include "optimise.h"
#include "types.h"
#include "surfel_io.h"
#include "utilities.h"
#include "depth_map_io.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "hierarchical_mesher_utilities.h"

const char pre_smooth_filename_template[] = "presmooth_%02d.bin";
const char post_smooth_filename_template[] = "smoothed_%02d.bin";

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
    using namespace spdlog;

    info("Loading properties");
    string property_file_name = (argc == 2) ? argv[1] : "animesh.properties";
    Properties properties{property_file_name};

    info("Loading depth maps");
    vector<DepthMap> depth_maps = load_depth_maps(properties);
    size_t num_frames = depth_maps.size();

    info("Loading cameras");
    vector<Camera> cameras = load_cameras(num_frames);

    info("Generating depth map hierarchy");
    vector<vector<DepthMap>> depth_map_hierarchy = create_depth_map_hierarchy(properties, depth_maps, cameras);
    maybe_save_depth_and_normal_maps(properties, depth_map_hierarchy);
    int num_levels = depth_map_hierarchy.size();

    // +-----------------------------------------------------------------------------------------------
    // | Construct Surfels for each level
    // +-----------------------------------------------------------------------------------------------
    size_t surfels_per_step = properties.getIntProperty("surfels-per-step");
    float convergence_threshold = properties.getFloatProperty("convergence-threshold");
    int current_level_index = num_levels - 1;
    vector<Surfel> previous_level_surfels;
    while (current_level_index >= 0) {
        info( "Generating surfels for level : {:d}", current_level_index);

        info( "   Getting correspondences");
        vector<vector<PixelInFrame>> correspondences = get_correspondences(properties, current_level_index,
                                                                           depth_map_hierarchy.at(current_level_index),
                                                                           cameras);

        info( "   Generating Surfels");
        vector<Surfel> current_level_surfels = generate_surfels(depth_map_hierarchy.at(current_level_index),
                                                                correspondences, properties);

        if (!previous_level_surfels.empty()) {
            initialise_tangents_from_previous_level(current_level_surfels, previous_level_surfels, properties);
        }

        info( "   Saving presmoothed Surfels");
        save_surfels_to_file(file_name_from_template_and_level(pre_smooth_filename_template,
                                                               current_level_index), current_level_surfels);

        info( "   Optimising");
        Optimiser o{convergence_threshold, num_frames, surfels_per_step};
        int cycles = o.optimise(current_level_surfels);
        info("Optimisation completed in {:d} cycles", cycles);

        info( "   Saving smoothed Surfels");
        save_surfels_to_file(file_name_from_template_and_level(post_smooth_filename_template,
                                                               current_level_index), current_level_surfels);

        // +-----------------------------------------------------------------------------------------------
        // | Remap surfels
        // +-----------------------------------------------------------------------------------------------
        previous_level_surfels = current_level_surfels;
        Surfel::surfel_by_id.clear();
        for (auto &s : previous_level_surfels) {
            Surfel::surfel_by_id.emplace(s.id, ref(s));
        }
        --current_level_index;
    }

    return 0;
}
