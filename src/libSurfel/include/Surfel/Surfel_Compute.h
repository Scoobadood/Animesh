#pragma once

#include <Eigen/Core>
#include <DepthMap/DepthMap.h>
#include <Properties/Properties.h>
#include <Graph/Graph.h>
#include "PixelInFrame.h"
#include "Surfel.h"
#include "SurfelGraph.h"

#include <vector>
#include <fstream>
#include <memory>

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
 * Return true if surfel1 and surfel2 are neighbours.
 * S1 is a neighbour of S2 iff:
 * S1 is represented in a frame F by pixel_in_frame P1 AND
 * S2 is represented in frame F by pixel_in_frame P2 AND
 * P1 and P2 are adjacent
 * @param surfel1 The first surfel to consider.
 * @param surfel2 The second surfel to consider.
 */
bool
are_neighbours(const std::shared_ptr<Surfel> &surfel1, const std::shared_ptr<Surfel> &surfel2, bool eight_connected);

/**
 * For a particular surfel, populate the list of neighbouring surfels.
 * A surfel Sn is a neighbour of another surfel S iff:
 * S is represented in a frame F by point P AND
 * Sn is represented in frame F by point Pn AND
 * P and Pn are neighbours.
 * The list of neighbours for a surfel is unique, that is, no matter how many frames
 * contain projections of S and Sn which are neighbours, Sn will occur only once in
 * the list of S's neighbours.
 * @param surfels The list of all surfels.
 * @param neighbours
 */
SurfelGraph
graph_from_surfels(std::vector<std::shared_ptr<Surfel>> &surfels, bool eight_connected);

SurfelGraph
generate_surfels(const std::vector<DepthMap> &depth_maps,
                 const std::vector<std::vector<PixelInFrame>> &correspondences,
                 const std::map<PixelInFrame, Eigen::Vector3f> &coordinates_by_pif,
                 const Properties &properties);

/**
 * Actually build a Surfel from the source data.
 */
std::shared_ptr<Surfel>
generate_surfel(const std::vector<PixelInFrame> &corresponding_pifs,
                const std::vector<DepthMap> &depth_maps,
                const std::map<PixelInFrame, Eigen::Vector3f> &coordinates_by_pif);
