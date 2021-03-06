//
// Created by Dave Durbin on 28/11/19.
//

#include <vector>
#include <iostream>
#include <DepthMap/DepthMap.h>
#include <DepthMap/DepthMapIO.h>
#include <Properties/Properties.h>
#include <Camera/Camera.h>
//#include "../../mesher/hierarchical_mesher_utilities.h"

const std::string CAMERA_TEMPLATE = "camera_F%02d.txt";

/**
 * Construct the save file name from a given template and level
 *
 */
std::string
file_name_from_template_and_level(const std::string &file_name_template, unsigned int level) {
    ssize_t bufsz = snprintf(nullptr, 0, file_name_template.c_str(), level);
    char file_name[bufsz + 1];
    snprintf(file_name, bufsz + 1, file_name_template.c_str(), level);
    return file_name;
}

/**
 * Construct the save file name from a given template and level and frame
 *
 */
std::string
file_name_from_template_level_and_frame(const std::string &file_name_template, unsigned int level, unsigned int frame) {
    // We expect 2x %2dL
    ssize_t bufsz = snprintf(nullptr, 0, file_name_template.c_str(), level, frame);
    char file_name[bufsz + 1];
    snprintf(file_name, bufsz + 1, file_name_template.c_str(), level, frame);
    return file_name;
}

/**
 * Construct a file name from a given template and frame
 *
 */
std::string
file_name_from_template_and_frame(const std::string &file_name_template, unsigned int frame) {
    // We expect %2dL
    ssize_t bufsz = snprintf(nullptr, 0, file_name_template.c_str(), frame);
    char file_name[bufsz + 1];
    snprintf(file_name, bufsz + 1, file_name_template.c_str(), frame);
    return file_name;
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
 * Load the cameras (one per frame)
 */
std::vector<Camera>
load_cameras(unsigned int num_frames) {
    using namespace std;

    std::vector<Camera> cameras;
    cameras.reserve(num_frames);
    cout << "Loading cameras " << flush;
    for (unsigned int i = 0; i < num_frames; ++i) {
        cout << "\rLoading cameras " << (i+1) << " of " << num_frames << "    " << flush;
        string file_name = file_name_from_template_and_frame(CAMERA_TEMPLATE, i);
        Camera camera = loadCameraFromFile(file_name);
        cameras.push_back(camera);
    }
    cout << endl;
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


tNormalMethod normal_computation_method(const Properties& properties ) {
    tNormalMethod method;
    std::string normal_method_name = properties.getProperty("normal-computation-method");
    if( normal_method_name == "pcl") {
        method = PCL;
    } else if ( normal_method_name == "cross-product") {
        method = CROSS;
    } else if ( normal_method_name == "planar") {
        method = PLANAR;
    } else {
        throw std::runtime_error("Unrecognised normal computation method ["+normal_method_name+"]");
    }
    return method;
}

void
maybe_save_depth_and_normal_maps(const Properties &properties,
                                 const std::vector<std::vector<DepthMap>> &depth_map_hierarchy) {
    using namespace std;

    int num_levels = depth_map_hierarchy.size();
    int num_frames = depth_map_hierarchy.at(0).size();

    if (properties.getBooleanProperty("dump-depth-maps")) {
        string dm_template = properties.getProperty("generated-depth-map-template");
        string norm_template = properties.getProperty("generated-normal-file-template");
        cout << " Dumping depth maps" << endl;
        for (unsigned int level = 0; level < num_levels; ++level) {
            for (unsigned int frame = 0; frame < num_frames; ++frame) {
                auto dm_file_name = file_name_from_template_level_and_frame(dm_template, level, frame);
                save_depth_map_as_pgm(dm_file_name, depth_map_hierarchy.at(level).at(frame));
                auto norm_file_name = file_name_from_template_level_and_frame(norm_template, level, frame);
                save_normals_as_ppm(norm_file_name, depth_map_hierarchy.at(level).at(frame));
            }
        }
    }
}

/**
 * Construct depth map hierarch given a vector of sourcde depth maps
 */
std::vector<std::vector<DepthMap>>
create_depth_map_hierarchy(const Properties &properties,
                           const std::vector<DepthMap> &depth_maps,
                           const std::vector<Camera> &cameras) {
    using namespace std;

    int num_levels = properties.getIntProperty("num-levels");
    cout << "Constructing depth map hierarchy with " << num_levels << " levels." << endl;

    vector<vector<DepthMap>> depth_map_hierarchy;
    depth_map_hierarchy.reserve(num_levels);
    depth_map_hierarchy.push_back(depth_maps);
    cout << "1 of " << num_levels << "    " << flush;
    for (int i = 1; i < num_levels; i++) {
        cout << "\r" << (i+1) << " of " << num_levels << "    " << flush;
        depth_map_hierarchy.push_back(resample_depth_maps(depth_map_hierarchy.at(i-1)));
    }
    cout << endl;

    tNormalMethod method = normal_computation_method(properties);

    //
    // Compute normals for each level
    for (auto & level_depth_maps : depth_map_hierarchy) {
        for (int f = 0; f < depth_maps.size(); ++f) {
            Camera camera = cameras.at(f);
            camera.set_image_size(level_depth_maps.at(0).width(), level_depth_maps.at(0).height());
            level_depth_maps.at(f).compute_normals(camera, method);
        }
    }

    maybe_save_depth_and_normal_maps(properties, depth_map_hierarchy);

    return depth_map_hierarchy;
}




