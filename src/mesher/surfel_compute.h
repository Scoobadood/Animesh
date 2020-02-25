#pragma once 

#include <vector>
#include <Eigen/Core>
#include <fstream>
#include "types.h"
#include "mesher_args.h"
#include <DepthMap/DepthMap.h>
#include <Properties/Properties.h>


/**
 * Return true if the two pixel in frames are neighbours.
 * They are neighbours if they are in the same frame and adjacent in an 8-connected
 * way. If they have the same coordinates in the frame they are NOT neighbours.
 * @param pif1 The first PixelinFrame.
 * @param pif2 The second PixelinFrame.
 */
bool
are_neighbours(const PixelInFrame &pif1, const PixelInFrame &pif2, bool use_eight_connected);

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

std::vector<Surfel>
generate_surfels(const std::vector<DepthMap> &depth_maps,
                 const std::vector<std::vector<PixelInFrame>> &correspondences,
                 const Properties& properties);
