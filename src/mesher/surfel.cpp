#ifdef DEBUG
#include <iostream>
#endif

#include <map>
#include <set>
#include <iostream>
#include "surfel.hpp"
#include <FileUtils/PgmFileParser.h>
#include <Geom/geom.h>

void 
populate_neighbours(std::vector<Surfel>& surfels, 
						 const std::vector<std::vector<std::vector<unsigned int>>>& 			neighbours,	
						 const std::map<std::pair<std::size_t, std::size_t>, std::size_t>&  frame_point_to_surfel) {
	using namespace std;

	for( int i=0; i<surfels.size(); ++i ) {
		for( auto fd : surfels.at(i).frame_data ) {
			unsigned int frame_idx = fd.frame_idx;
			unsigned int point_idx = fd.point_idx;

			vector<unsigned int> f_p_neighbours = neighbours.at(frame_idx).at(point_idx);
			for( auto n : f_p_neighbours) {
				size_t idx = frame_point_to_surfel.at(make_pair<>( frame_idx, n ) );
				surfels.at(i).neighbouring_surfels.push_back( idx );
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
		PointWithNormal pwn = point_normals.at(frame_idx).at(point_idx);
		fd.transform = vector_to_vector_rotation( y, pwn.normal );
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

/**
 * Write an unisgned int
 */
void 
write_unsigned_int( std::ofstream& file, unsigned int value ) {
    file.write( (const char *)&value, sizeof( unsigned int ) );
}
/**
 * Write a float
 */
void 
write_float( std::ofstream& file, float value ) {
    file.write( (const char *)&value, sizeof( float ) );
}
/**
 * Write an unisgned int
 */
void 
write_size_t( std::ofstream& file, std::size_t value ) {
    file.write( (const char *)&value, sizeof( std::size_t ) );
}

/**
 * Save surfel data as binary file to disk
 */
void 
save_to_file( const std::vector<Surfel>& surfels, 
			  const std::string& file_name ) {
	using namespace std;

    ofstream file{file_name, ios::out | ios::binary};
    write_unsigned_int( file, surfels.size());
    for( Surfel surfel : surfels) {
	    write_size_t( file, surfel.id);
    	write_unsigned_int( file, surfel.frame_data.size());
	    for( FrameData fd : surfel.frame_data) {
		    write_size_t( file, fd.frame_idx);
		    write_size_t( file, fd.point_idx);
		    write_float( file, fd.transform(0,0) );
		    write_float( file, fd.transform(0,1) );
		    write_float( file, fd.transform(0,2) );
		    write_float( file, fd.transform(1,0) );
		    write_float( file, fd.transform(1,1) );
		    write_float( file, fd.transform(1,2) );
		    write_float( file, fd.transform(2,0) );
		    write_float( file, fd.transform(2,1) );
		    write_float( file, fd.transform(2,2) );
	    }
    	write_unsigned_int( file, surfel.neighbouring_surfels.size());
	    for( auto idx : surfel.neighbouring_surfels) {
		    write_size_t( file, idx);
	    }
	    write_float( file, surfel.tangent.x() );
	    write_float( file, surfel.tangent.y() );
	    write_float( file, surfel.tangent.z() );
    }
    file.close();
}

unsigned int 
read_unsigned_int( std::ifstream& file ) {
	unsigned int i;
	file.read( (char *)&i, sizeof(i) );
	return i;
}

std::size_t
read_size_t( std::ifstream& file ) {
	size_t i;
	file.read( (char *)&i, sizeof(i) );
	return i;
}

float
read_float( std::ifstream& file ) {
	float i;
	file.read( (char *)&i, sizeof(i) );
	return i;
}

/**
 * Load surfel data from binary file
 */
void 
load_from_file( std::vector<Surfel>& surfels, 
			    const std::string& file_name ) {
	using namespace std;

	ifstream file{ file_name, ios::in | ios::binary};
	surfels.clear();

	unsigned int num_surfels = read_unsigned_int( file );
	cout << "  loading " << num_surfels << " surfels" << endl;
	int pct5 = num_surfels / 20;
	for( int sIdx=0; sIdx < num_surfels; ++sIdx ) {
		if( sIdx % pct5 == 0 ) {
			cout << "." << flush;
		}
		Surfel s;
		s.id = read_size_t(file);

		unsigned int num_frames = read_unsigned_int( file );
		for( int fdIdx = 0; fdIdx < num_frames; ++fdIdx ) {
			// cout << "      " << fdIdx << endl;
			FrameData fd;
			fd.frame_idx = read_size_t(file);
			fd.point_idx = read_size_t(file);
			float m[9];
			for( int mIdx=0; mIdx<9; mIdx++ ) {
				m[mIdx] = read_float(file);
			}
			fd.transform << m[0], m[1], m[2], m[3], m[4], m[5], m[6], m[7], m[8];
			s.frame_data.push_back( fd );
		}

		unsigned int num_neighbours = read_unsigned_int( file );
		for( int nIdx=0; nIdx<num_neighbours; ++nIdx) {
			s.neighbouring_surfels.push_back( read_size_t( file ) );
		}

		float v[3];
		for( int vIdx=0; vIdx < 3; ++vIdx ) {
			v[vIdx] = read_float(file);
		}
		s.tangent << v[0], v[1], v[2];

		surfels.push_back(s);
	}
	file.close();
	cout << endl;
}
