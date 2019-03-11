/*
 * Correspondence computation code
 * This code computes correspondences between images using 'cheat' data
 */

#pragma once 

#include <vector>

/**
 * Given a vector of vertex files, extract the correspondences between points in each file.
 * in this case using vertex correspondences identified in the files.
 * @param file_names The vertex files.
 * @return For each frame, for each point, a list of pairs of frame and point index for corresponding points
 * 		   in other frames.
 */
std::vector<std::vector<std::pair<unsigned int, unsigned int>>> 
compute_correspondences(const std::vector<std::string>& file_names);

