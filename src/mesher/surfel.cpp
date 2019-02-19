#pragma once

#ifdef DEBUG
#include <iostream>
#endif

#include <map>
#include <set>
#include "surfel.hpp"
#include <FileUtils/PgmFileParser.h>

std::vector<PixelLocation> find_pixel_neighbours(const PixelLocation& pl) {
	using namespace std;

	vector<PixelLocation> v;
	return v;
}

/*
For each correspondence set
	Construct a surfel
	For each corresponding pixel
		construct a frame data
	end
	Add to list
End

For each surfel
	For each corresponding pixel
		Find neighbours
		Lookup corr surfel
		Add to surfel nighbour list
		Compute normal
		Compute xform and add to frame data
	Done
End
 */


void
init_surfel_table(const std::vector<std::vector<PixelLocation>>& correspondences, 
					std::vector<Surfel>& surfels, 
					std::map<PixelLocation, unsigned int>& pixel_to_surfel ) {

	using namespace std;

	surfels.clear();
	for ( auto correspondence : correspondences ) {
		// Make a new Surfel
		Surfel s;
		s.id = surfels.size();

		// Add frame data for all pixels
		for ( auto pixel : correspondence) {
			FrameData frame_data;
			frame_data.pixel_location = pixel;
			// Transform is uninitialised
			s.frame_data.push_back(frame_data);
			pixel_to_surfel.insert(make_pair(pixel, s.id));
		}
		surfels.push_back(s);
	}
}

/*
 * Generally we want the 8-connected neighbours of this pixel
 * However it's possible that some of these pixels are not occipied or
 * have been removed from the frame as having dodgy accuracy.
 */
std::vector<PixelLocation> 
find_pixel_neighbours(const PixelLocation& pixel_location, const std::vector<Frame>& frames) {
	using namespace std;
	using namespace Eigen;

	vector<PixelLocation> neighbours;

	Frame frame = frames[pixel_location.frame_no];
	for( int dy = -1; dy <=1; ++dy ) {
		int ny = pixel_location.pixel_coord[1] + dy;
		for( int dx = -1; dx <=1; ++dx ) {
			int nx = pixel_location.pixel_coord[0] + dx;

			if( frame.isValidAt(nx, ny ) ) neighbours.push_back(
				PixelLocation{pixel_location.frame_no, 
					Vector2i{nx, ny}});
		}
	}

	return neighbours;
}

void
populate_surfel_data( std::vector<Surfel>& surfels, 
					  const std::map<PixelLocation, unsigned int>& pixel_to_surfel ) {
	using namespace std;

	// For all surfels
	for ( auto surfel : surfels ) {

		// For each pixel they're mapped to, find neighbours
		set<PixelLocation> all_neighbours;
		for ( auto fd : surfel.frame_data ) {
			// Find the pixel location
			auto pixel_location = fd.pixel_location;

			// Find neighbours
			vector<PixelLocation> neighbours = find_pixel_neighbours(pixel_location);

			// Compute the surface normal 
			all_neighbours.insert( neighbours.begin(), neighbours.end());
		}

		// For all neighbours in set
		for ( auto neighbour : all_neighbours) {
			auto it = pixel_to_surfel.find(neighbour);
			if ( it != pixel_to_surfel.end() ) {
				surfel.neighbouring_surfels.push_back(it->second);
			} else {
#ifdef DEBUG
				cerr << "Problem dereferencing surfel from pixel location: "
				     <<  pixel_location.frame_no << ":("
				     << pixel_location.pixel_coord[0]
				     << ", "
				     << pixel_location.pixel_coord[1]
				     << ")" << endl;
				exit(-1);
#endif
			}
		}
	}
}

void
build_surfel_table(const std::vector<std::vector<PixelLocation>>& correspondences, const std::vector<std::string>& files ) {
	using namespace std;

	vector<Surfel>  surfels;
	map<PixelLocation, unsigned int> pixel_to_surfel;
	init_surfel_table(correspondences, surfels, pixel_to_surfel);

	populate_surfel_data(surfels, pixel_to_surfel);
}
