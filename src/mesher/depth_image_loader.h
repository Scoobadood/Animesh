#pragma once

#include <string>
#include <vector>
#include <Eigen/Core>

struct PointWithNormal {
	Eigen::Vector3f point;
	Eigen::Vector3f normal;
};


/*
 * Load depth images from disk and convert to point couds.
 */
std::vector<std::vector<PointWithNormal>>
load_depth_images(const std::vector<std::string>& file_names);
