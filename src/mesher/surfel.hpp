#pragma once 

#include <vector>
#include <Eigen/Core>
#include "pixel_correspondence.hpp"

struct FrameData {
	PixelLocation	pixel_location;
	Eigen::Matrix3f	transform;
};

struct Surfel {
	std::size_t 			id;
	std::vector<FrameData>	frame_data;
	std::vector<size_t>		neighbouring_surfels;
	Eigen::Vector3f 		tangent;
};

void
build_surfel_table(const std::vector<std::vector<PixelLocation>>& correspondences, const std::vector<std::string>& files );