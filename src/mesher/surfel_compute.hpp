#pragma once 

#include <vector>
#include <Eigen/Core>
#include <fstream>
#include "types.h"
#include "mesher_args.h"

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
compute_surfels(const MesherArguments& args,
                std::vector<Surfel> &surfels);
