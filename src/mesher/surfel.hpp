#pragma once 

#include <vector>
#include <Eigen/Core>

struct FrameData {
	unsigned int 	frame_no;
	Eigen::Vector2f pixel_coord;
	Eigen::Matrix3f	transform;
};

struct Surfel {
	std::vector<FrameData>		frame_data;
	std::vector<unsigned int>	neighbouring_surfels;
	Eigen::Vector3f 			tangent;
};