/*
 * Correspondence computation code
 * This code computes correspondences between images using 'cheat' data
 */

#include <vector>
#include <string>
#include <Eigen/Core>

struct PixelLocation {
	std::size_t 	frame_no;
	Eigen::Vector2i pixel_coord;
};

std::vector<std::vector<PixelLocation>> compute_correspondences(const std::vector<std::string>& file_names);
