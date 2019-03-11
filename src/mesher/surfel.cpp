#ifdef DEBUG
#include <iostream>
#endif

#include <map>
#include <set>
#include "surfel.hpp"
#include <FileUtils/PgmFileParser.h>
#include <Geom/geom.h>

void 
populate_neighbours(std::vector<Surfel>& surfels, 
						 const std::vector<std::vector<std::vector<unsigned int>>>& 			neighbours,	
						 const std::map<std::pair<std::size_t, std::size_t>, std::size_t>&  frame_point_to_surfel) {
	using namespace std;

	for( int i=0; i<surfels.size(); ++i ) {
		for( auto fd : surfels[i].frame_data ) {
			unsigned int frame_idx = fd.frame_idx;
			unsigned int point_idx = fd.point_idx;

			vector<unsigned int> f_p_neighbours = neighbours[frame_idx][point_idx];
			for( auto n : f_p_neighbours) {
				size_t idx = frame_point_to_surfel.at(make_pair<>( frame_idx, n ) );
				surfels[i].neighbouring_surfels.push_back( idx );
			}
		}
	}
}

void
populate_frame_data( const std::vector<std::vector<PointWithNormal>>& point_normals,	// per frame, all point_normals
					 const std::vector<std::pair<unsigned int, unsigned int>>&  correspondence,
					 std::vector<FrameData>& frame_data) {
	using namespace Eigen;

	for( auto c : correspondence) {
		FrameData fd;
		unsigned int frame_idx = c.first;
		unsigned int point_idx = c.second;
		fd.frame_idx = frame_idx;
		fd.point_idx = point_idx;
		Vector3f y{ 0.0, 1.0, 0.0};
		fd.transform = vector_to_vector_rotation( y, point_normals[frame_idx][point_idx].normal );
		frame_data.push_back( fd );
	}
}



/**
 * Make the surfel table given a vector of points (with normals) for each frame
 * along with correspondences between them.
 * @param point_normals outer vector is the frame, inner vectr is the point normal.
 * @param neighbours Per frame, a lit of indices of the neighbours of a point where the index in the list matches the index in the point_normals list.
 * @param correspondences A vector of all correspondences where each correspondence is a vector of <frame,point_normal index>
 * @return A vector of surfels.
 */
std::vector<Surfel>
build_surfel_table(const std::vector<std::vector<PointWithNormal>>& point_normals,			// per frame, all point_normals
				   const std::vector<std::vector<std::vector<unsigned int>>>& neighbours,	// per frame, list of all points neighbouring
				   const std::vector<std::vector<std::pair<unsigned int, unsigned int>>>&  correspondences) 
{
	using namespace std;

	vector<Surfel> surfels;
	map<pair<size_t, size_t>, size_t> frame_point_to_surfel;

	for( auto correspondence : correspondences ) {
		Surfel surfel;

		surfel.id = surfels.size();
		populate_frame_data(point_normals, correspondence, surfel.frame_data);
		surfels.push_back( surfel );
		for( auto c : correspondence ) {
			frame_point_to_surfel.insert( make_pair<>(make_pair<>( c.first, c.second), surfel.id ));
		}
	}
	populate_neighbours(surfels, neighbours, frame_point_to_surfel);

	return surfels;
}