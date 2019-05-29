#pragma once

#include <string>
#include <vector>
#include <Eigen/Core>

struct PointWithNormal {
	Eigen::Vector3f point;
	Eigen::Vector3f normal;
};

struct PointWithNormal2_5D {
	Eigen::Vector2f point;
	float depth;
	Eigen::Vector3f normal;
};

/*
 * Load depth images from disk and convert to point couds.
 */
void
load_depth_images(	const std::vector<std::string>& 						file_names,
					std::vector<std::vector<PointWithNormal>>& 				point_clouds,
					std::vector<std::vector<std::vector<unsigned int>>>&	neighbours);

/**
 * Read depth images. Preprocess them by removing depths from disjunctions.
 */
void 
load_depth_images(const std::vector<std::string>& 						file_names,
				  std::vector<std::vector<PointWithNormal2_5D>>& 		point_clouds,
				  std::vector<std::vector<std::vector<unsigned int>>>&	neighbours);
