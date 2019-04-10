#ifdef DEBUG
#include <iostream>
#endif

#include <map>
#include <set>
#include <regex>
#include <random>
#include <iostream>
#include "surfel.hpp"
#include <FileUtils/PgmFileParser.h>
#include <Geom/geom.h>
#include "pixel_correspondence.hpp"
#include "depth_image_loader.h"

/*
	********************************************************************************
	**																			  **
	**					Load and Save    										  **
	**																			  **
	********************************************************************************
*/

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

/*
 * Write a vector
 */
void
write_vector_3f( std::ofstream& file, Eigen::Vector3f vector ) {
	write_float(file, vector.x());
	write_float(file, vector.y());
	write_float(file, vector.z());
}

/**
 * Save surfel data as binary file to disk
 */
/**
 * Save surfel data as binary file to disk
 */
void 
save_to_file( const std::string& file_name,
			  const std::vector<Surfel>& surfels, 
			  const std::vector<std::vector<PointWithNormal>>& point_normals)
{
	using namespace std;

    ofstream file{file_name, ios::out | ios::binary};

    unsigned int num_frames = point_normals.size();
    write_unsigned_int( file, num_frames);
    for( unsigned int i = 0; i < num_frames; ++i ) {
    	unsigned int num_points = point_normals.at(i).size();
	    write_unsigned_int( file, num_points );
	    for( unsigned int j = 0; j< num_points; ++j ) {
	    	write_vector_3f(file, point_normals.at(i).at(j).point);
	    	write_vector_3f(file, point_normals.at(i).at(j).normal);
	    }
    }

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
	    write_vector_3f(file, surfel.tangent);
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
	float value;
	file.read( (char *)&value, sizeof(float) );
	return value;
}

Eigen::Vector3f
read_vector_3f( std::ifstream& file ) {
	float x, y, z;
	file.read( (char *)&x, sizeof(float) );
	file.read( (char *)&y, sizeof(float) );
	file.read( (char *)&z, sizeof(float) );
	return Eigen::Vector3f{x, y, z};
}

/**
 * Load surfel data from binary file
 */

/**
 * Load surfel data from binary file
 */
void 
load_from_file( const std::string& file_name,
				std::vector<Surfel>& surfels, 
				std::vector<std::vector<PointWithNormal>>& point_normals)
{
	using namespace std;

	ifstream file{ file_name, ios::in | ios::binary};
	surfels.clear();
	point_normals.clear();

    unsigned int num_frames = read_unsigned_int(file);
    for( unsigned int i = 0; i<num_frames; ++i ) {
	    unsigned int num_points = read_unsigned_int( file);
	    vector<PointWithNormal> pwn;
	    for( unsigned int j = 0; j < num_points; ++j ) {
	    	Eigen::Vector3f point = read_vector_3f(file);
	    	Eigen::Vector3f normal = read_vector_3f(file);
	    	pwn.push_back(PointWithNormal{point, normal});
	    }
	    point_normals.push_back(pwn);
    }

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

		s.tangent = read_vector_3f( file );
		cout << "tan [" << sIdx << "]\t: " << s.tangent.x() << ", " << s.tangent.y() << ", " << s.tangent.z()   << endl;

		surfels.push_back(s);
	}
	file.close();
	cout << endl;
}


/*
	********************************************************************************
	**																			  **
	**					Build           										  **
	**																			  **
	********************************************************************************
*/


float 
random_zero_to_one( ) {
    static std::default_random_engine e;
    static std::uniform_real_distribution<> dis(0, 1); // rage 0 - 1
    return dis(e);
}

void 
randomize_tangents(std::vector<Surfel>& surfels) {
	for( auto iter = surfels.begin(); iter != surfels.end(); ++iter ) {
		float xc = random_zero_to_one( );
		float yc = sqrt(1.0f - (xc * xc));
		iter->tangent = Eigen::Vector3f{xc, 0.0f, yc};
	}
}

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
			set<unsigned int> set_of_neighbours;

			for( auto n : f_p_neighbours) {
				size_t idx = frame_point_to_surfel.at(make_pair<>( frame_idx, n ) );
				if( idx != surfels.at(i).id) {
					set_of_neighbours.insert(idx);
				}
			}
			// copy set into neighbouring_surfels
			copy(set_of_neighbours.begin(),
				 set_of_neighbours.end(),
				 back_inserter(surfels.at(i).neighbouring_surfels));
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
	randomize_tangents( surfels );
	return surfels;
}

std::vector<std::string> get_vertex_files_in_directory( std::string directory_name ) {
    using namespace std;

    vector<string> file_names;
    files_in_directory( directory_name, file_names, []( string name ) {
        using namespace std;

        std::transform(name.begin(), name.end(), name.begin(), ::tolower);

        const regex file_name_regex("\\/{0,1}(?:[^\\/]*\\/)*vertex_[0-9]+\\.pgm");
        return regex_match(name, file_name_regex);
    });
    // Construct full path names
    vector<string> full_path_names;
    for( string file_name : file_names) {
        // FIXME: Replace this evilness with something more robust and cross platform.
        string path_name = directory_name + "/" + file_name;
        full_path_names.push_back( path_name );
    }
    std::sort(full_path_names.begin(), full_path_names.end() );
    return full_path_names;
}

std::vector<std::string> get_depth_files_in_directory( std::string directory_name ) {
    using namespace std;

    vector<string> file_names;
    files_in_directory( directory_name, file_names, []( string name ) {
        using namespace std;

        std::transform(name.begin(), name.end(), name.begin(), ::tolower);

        const regex file_name_regex("\\/{0,1}(?:[^\\/]*\\/)*depth_[0-9]+\\.pgm");
        return regex_match(name, file_name_regex);
    });
    // Construct full path names
    vector<string> full_path_names;
    for( string file_name : file_names) {
        // FIXME: Replace this evilness with something more robust and cross platform.
        string path_name = directory_name + "/" + file_name;
        full_path_names.push_back( path_name );
    }
    std::sort(full_path_names.begin(), full_path_names.end() );
    return full_path_names;
}



void
load_from_directory(  const std::string& dir, 
                      std::vector<Surfel>& surfels, 
                      std::vector<std::vector<PointWithNormal>>& point_clouds ) 
{
  using namespace std;

  cout << "Computing correspondences..." << flush;
  vector<string> files = get_vertex_files_in_directory(dir);
  vector<vector<pair<unsigned int, unsigned int>>> correspondences;
  compute_correspondences(files, correspondences);
  cout << " done." << endl;

  cout << "Loading depth images..." << flush;
  files = get_depth_files_in_directory(dir);
  vector<vector<vector<unsigned int>>> neighbours;
  load_depth_images(files, point_clouds, neighbours);
  cout << " done." << endl;

  cout << "Building surfel table..." << flush;
  surfels = build_surfel_table(point_clouds, neighbours, correspondences);
  cout << " done." << endl;


  cout << "Saving..." << flush;
  save_to_file( "surfel_table.bin", surfels, point_clouds);
  cout << " done." << endl;
}