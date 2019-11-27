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
create_depth_map_hierarchy(const Properties &properties, const std::vector<DepthMap> &depth_maps);


/**
 * Load the cameras (one per frame)
 */
std::vector<Camera>
load_cameras(unsigned int num_frames);

#endif //ANIMESH_UTILITIES_H
