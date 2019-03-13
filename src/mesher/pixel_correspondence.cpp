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
 * Given a vector of vertex files, extract the correspondences between points in each file.
 * in this case using vertex correspondences identified in the files.
 * @param file_names The vertex files.
 * @return For each frame, for each point, a list of pairs of frame and point index for corresponding points
 * 		   in other frames.
 */
std::vector<std::vector<std::pair<unsigned int, unsigned int>>> 
compute_correspondences(const std::vector<std::string>& file_names)
{
	using namespace std;
	using namespace Eigen;

	multimap<unsigned int, pair<unsigned int, unsigned int>> vertex_to_frame_pixel;

	size_t current_frame_index = 0;
	for(auto file_name : file_names) {
		PgmData pgm = read_pgm(file_name);
		size_t source_pixel_idx = 0;
		size_t current_pixel_idx = 0;
		for( std::size_t y = 0; y<pgm.height; ++y ) {
			for( std::size_t x = 0; x < pgm.width; ++x ) {
				int vertex = pgm.data[source_pixel_idx];
				if( vertex != 0 ) {
					vertex_to_frame_pixel.insert( make_pair( vertex, make_pair(current_frame_index, current_pixel_idx)));
					current_pixel_idx++;
				}
				++source_pixel_idx;
			}
		}
		current_frame_index++;
	}

	// We now have a map from vertices to all corresponding frame/pixel pairs
	// We can transform this into a vector of vectors which can be used to 
	// derive surfel data.
	vector<vector<pair<unsigned int, unsigned int>>> correspondences;
	vector<pair<unsigned int, unsigned int>> correspondence;
	for (auto it = vertex_to_frame_pixel.begin(); it != vertex_to_frame_pixel.end(); ) {
		unsigned int vertex_id = it->first;
		do {
			unsigned int frame_idx = it->second.first;
			unsigned int pixel_idx = it->second.second;
			correspondence.push_back(make_pair(frame_idx, pixel_idx));
			++it;
		} while( (it != vertex_to_frame_pixel.end()) && (vertex_id == it->first));
		correspondences.push_back(correspondence);
		correspondence.clear();
	}

	return correspondences;
}
