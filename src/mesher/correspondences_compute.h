/**
 * Compute correspondences given a source directory.
 * Created by Dave Durbin on 2019-07-06.
 */

#ifndef ANIMESH_CORRESPONDENCES_COMPUTE_H
#define ANIMESH_CORRESPONDENCES_COMPUTE_H

#include <vector>
#include "types.h"
#include <DepthMap/DepthMap.h>

/**
 * Compute the correspondences between pixels in cleaned depth maps
 * Returns a vector or correspondences where each correspondence
 * is a PixelInFrame
 */
std::vector<std::vector<PixelInFrame>>
compute_correspondences(const std::vector<DepthMap>& depth_maps);

#endif //ANIMESH_CORRESPONDENCES_COMPUTE_H
