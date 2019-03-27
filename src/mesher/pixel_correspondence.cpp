/*
 * Correspondence computation code
 * This code computes correspondences between images using 'cheat' data
 */

#include <vector>
#include <map>
#include <iostream>
#include <Eigen/Core>
#include <FileUtils/PgmFileParser.h>
#include "pixel_correspondence.hpp"


/**
 * Given a vector of vertex files, extract the correspondences between points in each file.
 * in this case using vertex correspondences identified in the files.
 * @param file_names The vertex files.
 * @param correspondences Vector populated by this method with a list of pairs of frame and point index for corresponding points
 */
void
compute_correspondences(const std::vector<std::string>& file_names, 
						std::vector<std::vector<std::pair<unsigned int, unsigned int>>>& correspondences )
{
	using namespace std;
	using namespace Eigen;

	// For each frame, load each pixel and for each pixel assigned to a non-zero vertex
	// store the vertex to frame/pixel mapping.
	multimap<unsigned int, pair<unsigned int, unsigned int>> vertex_to_frame_pixel;
	size_t current_frame_idx = 0;
	for(auto file_name : file_names) {
		PgmData pgm = read_pgm(file_name);
		// cout << "Processing " << file_name << endl;

		size_t source_pixel_idx = 0;
		size_t current_pixel_idx = 0;
		for( std::size_t y = 0; y<pgm.height; ++y ) {
			for( std::size_t x = 0; x < pgm.width; ++x ) {
				int vertex = pgm.data.at(source_pixel_idx);
				// Ignore background
				if( vertex != 0 ) {
					// cout << "V:" << vertex << "==> (" << current_frame_idx << ", " << current_pixel_idx << ")" << endl;
					vertex_to_frame_pixel.insert( make_pair( vertex, make_pair(current_frame_idx, current_pixel_idx)));
					current_pixel_idx++;
				}
				++source_pixel_idx;
			}
		}
		// cout << "corr: frame " << current_frame_idx << " has " << current_pixel_idx << " pixels" << endl;
		current_frame_idx++;
	}

	// Report on vertex to frame pixel
	int num_frames = file_names.size();
	int frame_pixel_count[num_frames];
	for( int i=0; i<num_frames; ++i) {
		frame_pixel_count[i] = 0;
	}

	auto it = vertex_to_frame_pixel.begin();
	int vertex = it->first;
	int vertex_count = 0;
	while( it != vertex_to_frame_pixel.end() ) {
		if( it->first == vertex ) {
			frame_pixel_count[it->second.first]++;
			it++;
		} else {
			cout << "Vertex : " << vertex_count++ << "(" << vertex << ")" << ", Frame counts [";
			for( int i=0; i<num_frames; ++i) {
				cout << frame_pixel_count[i] << "  ";
				frame_pixel_count[i] = 0;
			}
			cout << "]" << endl;
			vertex = it->first;
		}
	}

	// We now have a map from vertices to all corresponding frame/pixel pairs
	// A correspondence is a vector of all frame/pixel pairs that have the same vertex
	correspondences.clear();
	vector<pair<unsigned int, unsigned int>> correspondence;
	int corr = 0;
	for (auto it = vertex_to_frame_pixel.begin(); it != vertex_to_frame_pixel.end(); ) {
		correspondence.clear();
		unsigned int vertex_id = it->first;
		do {
			correspondence.push_back(it->second);
			++it;
		} while( (it != vertex_to_frame_pixel.end()) && (vertex_id == it->first));
		cout << "Correspondence " << corr++ << "("<<vertex_id<<") : " << correspondence.size() << endl;
		correspondences.push_back(correspondence);
	}
}
