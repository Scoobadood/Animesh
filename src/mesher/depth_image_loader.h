#pragma once

#include <string>
#include <Eigen/Core>

struct PointWithNormal {
	Eigen::Vector3f point;
	Eigen::Vector3f normal;
};

/*
 * Load depth images from disk and convert to point couds.
 */
std::vector<PointWithNormal>
load_depth_image(const std::string& file_name);