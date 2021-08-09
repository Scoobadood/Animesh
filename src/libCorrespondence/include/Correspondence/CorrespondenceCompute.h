/**
 * Compute correspondences given a source directory.
 * Created by Dave Durbin on 2019-07-06.
 */

#pragma once

#include <vector>
#include <Surfel/PixelInFrame.h>
#include <DepthMap/DepthMap.h>
#include <Camera/Camera.h>

/**
 * Compute the correspondences between pixels in cleaned depth maps
 * Returns a vector or correspondences where each correspondence
 * is a PixelInFrame
 */
std::vector<std::vector<PixelInFrame>>
compute_correspondences(const std::vector<Camera> &cameras, const std::vector<DepthMap> &depth_maps);
