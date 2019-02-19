/*
 * Correspondence computation code
 * This code computes correspondences between images using 'cheat' data
 */

#pragma once 

#include <vector>
#include <string>
#include <Eigen/Core>

struct Frame {
	std::size_t 		width;
	std::size_t 		height;
	std::vector<int>	data;

	bool isValidAt( std::size_t x, std::size_t y ) {
		if( x > width || y > height ) return false;
		if( data[y * width + height] == 0 ) return false;
		return true;
	}
};

Frame load_frame_from_file( const std::string& file_name );


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
