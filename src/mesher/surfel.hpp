#pragma once 

#include <vector>
#include <Eigen/Core>
#include <fstream>
#include "pixel_correspondence.hpp"
#include "depth_image_loader.h"

struct PixelInFrame {
	unsigned int x;
	unsigned int y;
	unsigned int frame;

	PixelInFrame(unsigned int x, unsigned int y, unsigned int frame) :x{x}, y{y}, frame{frame}{};

	bool operator< (const PixelInFrame &other) {
		if( frame != other.frame)
			return frame < other.frame;

		if( y != other.y)
			return y < other.y;

		return x < other.x;
	}
};

struct FrameData {
	PixelInFrame	pixel_in_frame; // x, y, frame
	Eigen::Matrix3f	transform;			// Computed
};

struct Surfel {
	std::size_t 			id;
	std::vector<FrameData>	frame_data;
	std::vector<size_t>		neighbouring_surfels;
	Eigen::Vector3f 		tangent;
};

/**
 * Make the surfel table given a vector of points (with normals) for each frame
 * along with correspondences between them.
 * @param point_normals outer vector is the frame, inner vectr is the point normal.
 * @param neighbours Per frame, a lit of indices of the neighbours of a point where the index in the list matches the index in the point_normals list.
 * @param correspondences A vector of all correspondences where each correspondence is a vector of <frame,point_normal index>
 * @return A vector of surfels.
 */
std::vector<Surfel>
build_surfel_table(const std::vector<std::vector<PointWithNormal2_5D>>& point_normals,			// per frame, all point_normals
				   const std::vector<std::vector<std::vector<unsigned int>>>& neighbours,	// per frame, list of all points neighbouring
				   const std::vector<std::vector<std::pair<unsigned int, unsigned int>>>&  correspondences);


/**
 * Initialie all tangents to random values
 */
void 
randomize_tangents(std::vector<Surfel>& surfels);

/**
 * Save surfel data as binary file to disk
 */
void 
save_to_file( const std::string& file_name,
			  const std::vector<Surfel>& surfels);

/**
 * Load surfel data from binary file
 */
void 
load_from_file( const std::string& file_name,
				std::vector<Surfel>& surfels);

void
load_from_directory(  const std::string& dir, 
                      std::vector<Surfel>& surfels);
