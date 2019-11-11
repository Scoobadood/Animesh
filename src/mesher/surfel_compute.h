#pragma once 

#include <vector>
#include <Eigen/Core>
#include <fstream>
#include "types.h"
#include "mesher_args.h"
#include <DepthMap/DepthMap.h>

/**
 * Sort all framedata for each surfel in ascending order of frame id.
 * We do this once to facilitate finding common frames.
 */
void 
sort_frame_data(std::vector<Surfel>& surfels);

/**
 * Initialise all tangents to random values
 */
void 
randomize_tangents(std::vector<Surfel>& surfels);

void
generate_surfels(const std::vector<DepthMap> &depth_maps,
                 const std::vector<std::vector<const PixelInFrame>> &correspondences,
                 std::vector<Surfel> &surfels);
