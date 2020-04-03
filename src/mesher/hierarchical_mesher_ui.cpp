//
// Created by Dave Durbin on 26/3/20.
//

#include <iostream>

#include <nanogui/nanogui.h>
#include "AnimeshApplication.h"

int main(int argc, char ** argv) {
    using namespace std;
    try {
        nanogui::init();
        /* scoped variables */
        {
            nanogui::ref <AnimeshApplication> app = new AnimeshApplication(argc, argv);
            app->drawAll();
            app->setVisible(true);
            nanogui::mainloop();
        }
        nanogui::shutdown();
    } catch (const std::runtime_error &e) {
        string error_msg = string("Caught a fatal error: ") + string(e.what());
        cerr << error_msg << endl;
        return -1;
    }
    return 0;
}

/**
 * Entry point
 * Generate a vector of Surfels and save to disk.
 *
 * @param argc
 * @param argv
 * @return
 */


//    info("Loading properties");
//    string property_file_name = (argc == 2) ? argv[1] : "animesh.properties";
//    Properties properties{property_file_name};
//
//    info("Loading depth maps");
//    vector<DepthMap> depth_maps = load_depth_maps(properties);
//    size_t num_frames = depth_maps.size();
//
//    info("Loading cameras");
//    vector<Camera> cameras = load_cameras(num_frames);
//
//    info("Generating depth map hierarchy");
//    vector<vector<DepthMap>> depth_map_hierarchy = create_depth_map_hierarchy(properties, depth_maps, cameras);
//    maybe_save_depth_and_normal_maps(properties, depth_map_hierarchy);
//    int num_levels = depth_map_hierarchy.size();
//
//    // +-----------------------------------------------------------------------------------------------
//    // | Construct Surfels for each level
//    // +-----------------------------------------------------------------------------------------------
//    size_t surfels_per_step = properties.getIntProperty("surfels-per-step");
//    float convergence_threshold = properties.getFloatProperty("convergence-threshold");
//    int current_level_index = num_levels - 1;
//    vector<Surfel> previous_level_surfels;
//    while (current_level_index >= 0) {
//        info( "Generating surfels for level : {:d}", current_level_index);
//
//        info( "   Getting correspondences");
//        vector<vector<PixelInFrame>> correspondences = get_correspondences(properties, current_level_index,
//                                                                           depth_map_hierarchy.at(current_level_index),
//                                                                           cameras);
//
//        info( "   Generating Surfels");
//        vector<Surfel> current_level_surfels = generate_surfels(depth_map_hierarchy.at(current_level_index),
//                                                                correspondences, properties);
//
//        if (!previous_level_surfels.empty()) {
//            initialise_tangents_from_previous_level(current_level_surfels, previous_level_surfels, properties);
//        }
//
//        info( "   Saving presmoothed Surfels");
//        save_surfels_to_file(file_name_from_template_and_level(pre_smooth_filename_template,
//                                                               current_level_index), current_level_surfels);
//
//        info( "   Optimising");
//        Optimiser o{convergence_threshold, num_frames, surfels_per_step};
//        int cycles = o.optimise(current_level_surfels);
//        info("Optimisation completed in {:d} cycles", cycles);
//
//        info( "   Saving smoothed Surfels");
//        save_surfels_to_file(file_name_from_template_and_level(post_smooth_filename_template,
//                                                               current_level_index), current_level_surfels);
//
//        // +-----------------------------------------------------------------------------------------------
//        // | Remap surfels
//        // +-----------------------------------------------------------------------------------------------
//        previous_level_surfels = current_level_surfels;
//        Surfel::surfel_by_id.clear();
//        for (auto &s : previous_level_surfels) {
//            Surfel::surfel_by_id.emplace(s.id, ref(s));
//        }
//        --current_level_index;
//    }
//
//    return 0;
//}
