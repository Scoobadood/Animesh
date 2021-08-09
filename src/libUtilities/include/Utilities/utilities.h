//
// Created by Dave Durbin on 28/11/19.
//

#ifndef ANIMESH_UTILITIES_H
#define ANIMESH_UTILITIES_H

#include <vector>
#include <DepthMap/DepthMap.h>
#include <Properties/Properties.h>

std::vector<DepthMap>
load_depth_maps(const Properties &properties);

/**
 * Take the vector of depth maps and subsample each, returnomng a new vector of the sumbsampled maps.
 * @param depth_maps The input vector
 * @return A new vector of submsampled maps.
 */
std::vector<DepthMap>
resample_depth_maps(const std::vector<DepthMap> &depth_maps);

/**
 * Construct depth map hierarch given a vector of sourcde depth maps
 */
std::vector<std::vector<DepthMap>>
create_depth_map_hierarchy(const Properties &properties,
        const std::vector<DepthMap> &depth_maps,
        const std::vector<Camera> &cameras);


/**
 * Load the cameras (one per frame)
 */
std::vector<Camera>
load_cameras(unsigned int num_frames);


/**
 * Construct the save file name from a given template and level
 *
 */
std::string
file_name_from_template_and_level(const std::string &file_name_template, unsigned int level);

/**
 * Construct the save file name from a given template and level and frame
 *
 */
std::string
file_name_from_template_level_and_frame(const std::string &file_name_template, unsigned int level, unsigned int frame);

/**
 * Construct a file name from a given template and frame
 *
 */
std::string
file_name_from_template_and_frame(const std::string &file_name_template, unsigned int frame);


void
maybe_save_depth_and_normal_maps(const Properties &properties,
                                 const std::vector<std::vector<DepthMap>> &depth_map_hierarchy);

#endif //ANIMESH_UTILITIES_H
