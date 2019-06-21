#pragma once 

#include <vector>
#include <Eigen/Core>
#include <fstream>
#include "depth_image_loader.h"

struct PixelInFrame {
	unsigned int x;
	unsigned int y;
	unsigned int frame;

	PixelInFrame(unsigned int x, unsigned int y, unsigned int frame) :x{x}, y{y}, frame{frame}{};

	bool operator< (const PixelInFrame &other) const {
		if( frame != other.frame)
			return frame < other.frame;

		if( y != other.y)
			return y < other.y;

		return x < other.x;
	}
};

struct FrameData {
	PixelInFrame	pixel_in_frame; 	// x, y, frame
	Eigen::Matrix3f	transform;			// Computed
	Eigen::Vector3f normal;				// Normal at pixel in frame
	FrameData(const PixelInFrame& pif, const Eigen::Matrix3f& tran, const Eigen::Vector3f& norm) : pixel_in_frame{pif}, transform{tran}, normal{norm}
	{}

	FrameData() : pixel_in_frame{0, 0, 0}, transform{Eigen::Matrix3f::Identity()}, normal{Eigen::Vector3f::Zero()}
	{}
};

struct Surfel {
	std::size_t 			id;
	std::vector<FrameData>	frame_data;
	std::vector<size_t>		neighbouring_surfels;
	Eigen::Vector3f 		tangent;
};

/**
 * Sort all framedata for each surfel in ascending order of frame id.
 * We do this once to facilitate finding common frames.
 */
void 
sort_frame_data(std::vector<Surfel>& surfels);

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
