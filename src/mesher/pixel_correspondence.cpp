/*
 * Correspondence computation code
 * This code computes correspondences between images using 'cheat' data
 */

#include <vector>
#include <map>
#include <Eigen/Core>
#include <FileUtils/PgmFileParser.h>
#include "pixel_correspondence.hpp"

/**
 * @return a vector of vectors of pixel locations. Each entry in the vector corresponds to a distinct 'surfel'
 * each vector lists the fram/pixel coordinates of points correspondning to this surfel.
 */
std::vector<std::vector<PixelLocation>> compute_correspondences(const std::vector<std::string>& file_names) {
	using namespace std;
	using namespace Eigen;

	multimap<size_t, PixelLocation> vertex_to_pixels;

	size_t current_frame_index = 0;
	for(auto file_name : file_names) {
		Frame frame = load_frame_from_file( file_name );
		size_t idx = 0;
		for( std::size_t y = 0; y<frame.height; ++y ) {
			for( std::size_t x = 0; x < frame.width; ++x ) {
				int vertex = frame.data[idx];
				PixelLocation pl{ current_frame_index, Vector2i{x, y}};
				vertex_to_pixels.insert( make_pair( vertex, pl));
			}
		}
		current_frame_index++;
	}

	// We now have a map from vertices to all corresponding pixels.
	// We can transform this into a vector of vectors which can be used to 
	// derive surfel data.

	vector<vector<PixelLocation>> correspondences;
	vector<PixelLocation> correspondence;
	for (auto it = vertex_to_pixels.begin(); it != vertex_to_pixels.end(); ) {
		size_t vertex_id = it->first;
		do {
			correspondence.push_back(it->second);
			++it;
		} while( (it != vertex_to_pixels.end()) && (vertex_id == it->first));
		correspondences.push_back(correspondence);
		correspondence.clear();
	}
	return correspondences;
}
