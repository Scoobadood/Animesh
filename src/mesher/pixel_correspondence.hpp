/*
 * Correspondence computation code
 * This code computes correspondences between images using 'cheat' data
 */

#pragma once 

#include <vector>
#include <string>
#include <Eigen/Core>

struct PixelLocation {
	std::size_t 	frame_no;
	Eigen::Vector2i pixel_coord;
	bool operator<(const PixelLocation& other) const {
		if(frame_no < other.frame_no) {
			return true;
		}
		if(frame_no != other.frame_no) {
			return false;
		}
		if( pixel_coord[0] < other.pixel_coord[0])  {
			return true;
		}
		if( pixel_coord[0] != other.pixel_coord[0])  {
			return false;
		}
		if( pixel_coord[1] != other.pixel_coord[1])  {
			return true;
		}
		return false;
	}
};

std::vector<std::vector<PixelLocation>> compute_correspondences(const std::vector<std::string>& file_names);
