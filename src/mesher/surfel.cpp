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
#include <DepthMap/DepthMap.h>
#include <Geom/geom.h>
#include "depth_image_loader.h"

static const char * DEPTH_FILE_NAME_REGEX = "\\/{0,1}(?:[^\\/]*\\/)*depth_[0-9]+\\.mat";
static const char * VERTEX_FILE_NAME_REGEX = "\\/{0,1}(?:[^\\/]*\\/)*vertex_[0-9]+\\.pgm";

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
write_vector_2f( std::ofstream& file, Eigen::Vector2f vector ) {
	write_float(file, vector.x());
	write_float(file, vector.y());
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
			  const std::vector<Surfel>& surfels)
{
	using namespace std;

    ofstream file{file_name, ios::out | ios::binary};

    // Count
    write_unsigned_int( file, surfels.size());
    for( auto const & surfel : surfels) {
    	// ID
	    write_size_t( file, surfel.id);
	    // FrameData size
    	write_unsigned_int( file, surfel.frame_data.size());
	    for( auto const &  fd : surfel.frame_data) {
	    	// PixelInFrame
		    write_size_t( file, fd.x);
		    write_size_t( file, fd.y);
		    write_size_t( file, fd.frame);
		    // Transform
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

Eigen::Vector2f
read_vector_2f( std::ifstream& file ) {
	float x, y;
	file.read( (char *)&x, sizeof(float) );
	file.read( (char *)&y, sizeof(float) );
	return Eigen::Vector2f{x, y};
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
void 
load_from_file( const std::string& file_name,
				std::vector<Surfel>& surfels, 
				std::vector<std::vector<PointWithNormal2_5D>>& point_normals)
{
	using namespace std;

	ifstream file{ file_name, ios::in | ios::binary};
	surfels.clear();
	point_normals.clear();

	unsigned int num_surfels = read_unsigned_int( file );
	cout << "  loading " << num_surfels << " surfels" << endl;
	int pct5 = num_surfels / 20;
	for( int sIdx=0; sIdx < num_surfels; ++sIdx ) {
		if( sIdx % pct5 == 0 ) {
			cout << "." << flush;
		}
		Surfel s;
		s.id = read_size_t(file);

		num_frames = read_unsigned_int( file );
		for( int fdIdx = 0; fdIdx < num_frames; ++fdIdx ) {
			FrameData fd;

	    	// PixelInFrame
		    fd.x = read_size_t(file);
		    fd.y = read_size_t(file);
		    fd.frame = read_size_t(file);

		    // Transform
			float m[9];
			for( int mIdx = 0; mIdx<9; mIdx++ ) {
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
		surfels.push_back(s);
	}
	file.close();
	cout << endl;
    std::cout << "in load from file [66]: " << surfels.at(66).tangent.x() << ", " << surfels.at(66).tangent.y() << ", " <<  surfels.at(66).tangent.z() << std::endl;
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
	for( auto & surfel : surfels) {
		float xc = random_zero_to_one( );
		float yc = sqrt(1.0f - (xc * xc));
        surfel.tangent = Eigen::Vector3f{xc, 0.0f, yc};
	}
}

void 
populate_neighbours(std::vector<Surfel>& surfels, 
						 const std::vector<std::vector<std::vector<unsigned int>>>& 			neighbours,	
						 const std::map<std::pair<std::size_t, std::size_t>, std::size_t>&  frame_point_to_surfel) {
	using namespace std;

	for( auto & surfel : surfels ) {
		for( auto const & fd : surfel.frame_data ) {
			unsigned int frame_idx = fd.frame_idx;
			unsigned int point_idx = fd.point_idx;

			vector<unsigned int> f_p_neighbours = neighbours.at(frame_idx).at(point_idx);
			set<unsigned int> set_of_neighbours;

			for( auto n : f_p_neighbours) {
				size_t idx = frame_point_to_surfel.at(make_pair<>( frame_idx, n ) );
				if( idx != surfel.id) {
					set_of_neighbours.insert(idx);
				}
			}
			// copy set into neighbouring_surfels
			copy(set_of_neighbours.begin(),
				 set_of_neighbours.end(),
				 back_inserter(surfel.neighbouring_surfels));
		}
	}
}

/**
 * For each entry in the correspondence group, we need to construct a 
 * FrameData object which tells us the frame and pixel affected as
 * well as the required transform.
 */
void
populate_frame_data( const std::vector<PixelInFrame>&	correspondence_group,
					 std::vector<DepthMap>& 			depth_maps,
					 std::vector<FrameData>& 			frame_data) {
	using namespace std;
	using namespace Eigen;

	for( auto c : correspondence_group) {
		FrameData fd;
		fd.pixel_in_frame = c;
		Vector3f y{ 0.0, 1.0, 0.0};
		Vector3f target_normal = depth_maps.at(frame_idx).normal_at(c.y, c.x);
		fd.transform = vector_to_vector_rotation( y, pwn.target_normal );
		frame_data.push_back( fd );
	}
}

inline std::string file_in_directory(const std::string& directory, const std::string& file ) {
    // FIXME: Replace this evilness with something more robust and cross platform.
    std::string full_path = directory;
    full_path .append("/").append(file);
    return full_path;
}

std::vector<std::string> get_vertex_files_in_directory( const std::string& directory_name ) {
    using namespace std;

    vector<string> file_names;
    files_in_directory( directory_name, file_names, []( string name ) {
        using namespace std;

        std::transform(name.begin(), name.end(), name.begin(), ::tolower);

        const regex file_name_regex(VERTEX_FILE_NAME_REGEX);
        return regex_match(name, file_name_regex);
    });
    // Construct full path names
    vector<string> full_path_names;
    for( auto const & file_name : file_names) {
        string path_name = file_in_directory(directory_name, file_name);
        full_path_names.push_back( path_name );
    }
    std::sort(full_path_names.begin(), full_path_names.end() );
    return full_path_names;
}

std::vector<std::string> get_depth_files_in_directory( const std::string& directory_name ) {
    using namespace std;

    vector<string> file_names;
    files_in_directory( directory_name, file_names, []( string name ) {
        using namespace std;

        std::transform(name.begin(), name.end(), name.begin(), ::tolower);

        const regex file_name_regex(DEPTH_FILE_NAME_REGEX);
        return regex_match(name, file_name_regex);
    });
    // Construct full path names
    vector<string> full_path_names;
    for( auto const & file_name : file_names) {
        string path_name = file_in_directory(directory_name, file_name);
        full_path_names.push_back( path_name );
    }
    std::sort(full_path_names.begin(), full_path_names.end() );
    return full_path_names;
}



/*
	********************************************************************************
	**																			  **
	**                                 Top Level Calls                            **
	**																			  **
    ********************************************************************************
*/


/**
 * Load the depth maps from a list of files into memory
 * as a vector.
 */
void
load_depth_maps(const std::string& source_directory, std::vector<DepthMap> depth_maps) {
	std::vector<std::string> files = get_depth_files_in_directory(source_directory);
  	if( files.size() == 0 ) {
  		throw std::runtime_error( "No depth images found in " + source_directory);
  	}

  	depth_maps.clear();
  	for( auto file_name : files ) {
  		DepthMap dm{file_name};
  		depth_maps.push_back(dm);
  	}
}


/**
 * Preprocess depth maps to 
 * - remove noise
 * - remove dubious data (ie edge points)
 */
void 
preprocess_depth_maps(std::vector<DepthMap>& depth_maps) {
	const float TS = 8.0f;
	const float TL = 3.0f;
	for( auto dm : depth_maps ) {
		dm.cull_unreliable_depths(TS, TL);
		dm.get_normals();
	}
}

/**
 * Compute the correspondences between pixels in cleaned depth maps
 * Returns a vector or correspondences where each correspondence 
 * is a PixelInFrame
 */
void 
compute_correspondences(const std::string& source_directory, 
						const std::vector<DepthMap>& depth_maps, 
						std::vector<std::vector<PixelInFrame>>& correspondences) {
	using namespace std;
	using namespace Eigen;

	// TODO: Ultimately this will be handled more elegantly and less cheatily
	vector<string> file_names = get_vertex_files_in_directory(source_directory);
	assert(file_names.size() == depth_maps.size());

	// For each frame, load each pixel and for each pixel assigned to a non-zero vertex
	// WHERE the pixel has not been culled from a depth map, i.e. it has a legitimate normal
	// NOTE This code depends critically on there being the same number of vertex files as depth files and
	// that they are ordered in the same way, ie vertex file[0] corresponds to depth_map[0]

	multimap<unsigned int, PixelInFrame> vertex_to_frame_pixel;
	size_t current_frame_idx = 0;
	for(auto file_name : file_names) {
		PgmData pgm = read_pgm(file_name);

		size_t source_pixel_idx = 0;
		for( std::size_t row = 0; row < pgm.height; ++row ) {
			for( std::size_t col = 0; col < pgm.width; ++col ) {
				int vertex = pgm.data.at(source_pixel_idx);
				// Ignore background nd undefined normals
				if( vertex != 0 && depth_maps[current_frame_idx].is_normal_defined(row, col) ) {
					vertex_to_frame_pixel.insert( make_pair( vertex, PixelInFrame(col, row, current_frame_idx)));
				}
				++source_pixel_idx;
			}
		}
		current_frame_idx++;
	}


	// We now have a map from vertices to all corresponding frame/pixel pairs
	// A correspondence is a vector of all frame/pixel pairs that have the same vertex
	correspondences.clear();
	vector<PixelInFrame> correspondence;
	int corr = 0;
	for (auto it = vertex_to_frame_pixel.begin(); it != vertex_to_frame_pixel.end(); ) {
		correspondence.clear();
		unsigned int vertex_id = it->first;
		do {
			correspondence.push_back(it->second);
			++it;
		} while( (it != vertex_to_frame_pixel.end()) && (vertex_id == it->first));
		correspondences.push_back(correspondence);
	}
}


/**
 * Generate surfel and pointcloud data from depth images
 * given correspondences.
 */

/*
	For each correspondence
		Make a surfel
		-- For each Frame/Pixel mapping from that correspondence compute the one we will use
		-- Compute the normal (or get it)
		-- Cmpute the transformation matrix
		-- Store the Frame Data
		-- Init the random direction vector.
 */
void 
generate_surfels(const std::vector<DepthMap>& 					depth_maps,
				 const std::vector<std::vector<PixelInFrame>>&	correspondences,
				 std::vector<Surfel>& 							surfels) 
{
	using namespace std;

	surfels.clear();
	point_clouds.clear();

	// One per correpondence
	// For each one, add point, depth, normal
	map<PixelInFrame, size_t> pixel_in_frame_to_surfel;

	for( auto const & correspondence_group : correspondences ) {
		Surfel surfel;

		surfel.id = surfels.size();
		populate_frame_data(correspondence_group, depth_maps, surfel.frame_data);
		surfels.push_back( surfel );
		for( auto c : correspondence_group ) {
			pixel_in_frame_to_surfel.insert( make_pair<>(c, surfel.id ));
		}
	}
	populate_neighbours(surfels, neighbours, pixel_in_frame_to_surfel);
	randomize_tangents( surfels );
}

/*
	********************************************************************************
	**																			  **
	**                                 Entry Point                                **
	**																			  **
	********************************************************************************
*/


/*
	Load and clean depth maps
	--> Output DepthMap objects
	--> Compute normals
	--> Sets up is_normal_valid_at()

	Load correspondences (from depth maps)
	--> Load vectors,
	--> Construct map from framepixel to vertex (If normal is valid)

	Make correspondences from map

	For each correspondence
		Make a surfel
		-- For each Frame/Pixel mapping from that correspondence compute the one we will use
		-- Compute the normal (or get it)
		-- Cmpute the transformation matrix
		-- Store the Frame Data
		-- Init the random direction vector.
*/
void
load_from_directory(  const std::string& dir, 
                      std::vector<Surfel>& surfels ) 
{
  using namespace std;

  cout << "Reading depth maps ..." << flush;
  vector<DepthMap> depth_maps;
  load_depth_maps(dir, depth_maps);

  cout << "Preprocessing depth maps ..." << flush;
  preprocess_depth_maps(depth_maps);

  cout << "Computing correspondences..." << flush;
  vector<vector<PixelInFrame>> correspondences;
  compute_correspondences(dir, depth_maps, correspondences);

  cout << "Generating surfels..." << flush;
  generate_surfels(depth_maps, correspondences, surfels);

  cout << " done." << endl;
  // cout << "Processing " << files.size() << " depth images..." << flush;
  // process_depth_files(files, surfels, point_clouds)
}