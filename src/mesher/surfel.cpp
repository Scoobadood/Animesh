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

static const char * DEPTH_FILE_NAME_REGEX = "\\/{0,1}(?:[^\\/]*\\/)*depth_[0-9]+\\.mat";
static const char * VERTEX_FILE_NAME_REGEX = "\\/{0,1}(?:[^\\/]*\\/)*vertex_[0-9]+\\.pgm";

/*
	********************************************************************************
	**																			  **
	**					Utilities        										  **
	**																			  **
	********************************************************************************
*/
bool compare_frame_data_by_frame(const FrameData& fd1, const FrameData& fd2) {
    return fd1.pixel_in_frame.frame < fd2.pixel_in_frame.frame;
}
/**
 * Sort all framedata for each surfel in ascending order of frame id.
 * We do this once to facilitate finding common frames.
 */
void 
sort_frame_data(std::vector<Surfel>& surfels) {
	for( auto surfel : surfels ) {
		sort(surfel.frame_data.begin(), surfel.frame_data.end(), compare_frame_data_by_frame);
	}
}



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

    cout << "Saving..." << flush;

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
		    write_size_t( file, fd.pixel_in_frame.x);
		    write_size_t( file, fd.pixel_in_frame.y);
		    write_size_t( file, fd.pixel_in_frame.frame);
			write_float(file, fd.depth);

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

			// Normal
			write_vector_3f( file, fd.normal );
	    }
    	write_unsigned_int( file, surfel.neighbouring_surfels.size());
	    for( auto idx : surfel.neighbouring_surfels) {
		    write_size_t( file, idx);
	    }
	    write_vector_3f(file, surfel.tangent);
	}
    file.close();
    cout << " done." << endl;
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
load_from_file( const std::string& file_name, std::vector<Surfel>& surfels)
{
	using namespace std;

	cout << "Loading from file " << file_or_directory << "..." << flush;

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
			FrameData fd;

	    	// PixelInFrame
		    fd.pixel_in_frame.x = read_size_t(file);
		    fd.pixel_in_frame.y = read_size_t(file);
		    fd.pixel_in_frame.frame = read_size_t(file);
		    fd.depth = read_float(file);

		    // Transform
			float m[9];
			for( int mIdx = 0; mIdx<9; mIdx++ ) {
				m[mIdx] = read_float(file);
			}
			fd.transform << m[0], m[1], m[2], m[3], m[4], m[5], m[6], m[7], m[8];

			// Normal
			fd.normal = read_vector_3f( file );
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
	cout << endl << " done." << endl;
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

/**
 * Return true if the two pixel in frames are neighbours.
 * They are neighbours if they are in the same frame and adjacent in an 8-connected
 * way. If they have the same coordinates in the frame they aree NOT neighbours.
 * @param pif1 The first PixelinFrame.
 * @param pif2 The second PixelinFrame.
 */
bool
are_neighbours(const PixelInFrame& pif1, const PixelInFrame& pif2) {
	if( pif1.frame != pif2.frame) {
		return false;
	}
	int dx = (int)pif1.x - (int)pif2.x;
	int dy = (int)pif1.y - (int)pif2.y;
	if( dx == 0 && dy == 0 ) {
		return false;
	}
	if( std::abs(dx) <= 1 && std::abs(dy) <=1 ) {
		return true;
	}
	return false;
}

/**
 * Return true if surfel1 and surfel2 are neighbours.
 * S1 is a neighbour of S2 iff:
 * S1 is represented in a frame F by point P1 AND
 * S2 is represented in frame F by point P2 AND
 * P1 and P2 are adjacent in an 8-connected way
 * // TODO: Consider depth disparities. We may have eliminated this as a problem during depth map cleanup but perhaps not.
 * @param surfel1 The first surfel to consider.
 * @param surfel2 The second surfel to consider.
 */
bool 
are_neighbours(const Surfel& surfel1, const Surfel& surfel2 ) {
	using namespace std;

	// Sort framedata for each surfel by frame index
	// TODO: This is an expensive shallow copy operation. We should probably avoid it.
	vector<FrameData> fd1 = surfel1.frame_data;
	sort(fd1.begin(), fd1.end(), compare_frame_data_by_frame);
	vector<FrameData> fd2 = surfel2.frame_data;
	sort(fd2.begin(), fd2.end(), compare_frame_data_by_frame);

	// While both lists have a frame left
	//   if frame at front of both lists are same
	//     if points in that frame are neighbours, 
	//       return true
	//     else
	//       discard both frames
	//   else
	//     discard lower frame
	//   endif
	// endwhile
	// return false
	auto it1 = fd1.begin();
	auto it2 = fd2.begin();
	while( ( it1 != fd1.end() ) && ( it2 != fd2.end() ) ) {
		PixelInFrame pif1 = it1->pixel_in_frame;
		PixelInFrame pif2 = it2->pixel_in_frame;
		if( pif1.frame == pif2.frame) {
			if( are_neighbours(pif1, pif2)) {
				return true;
			} else {
				++it1;
				++it2;
			}
		} else if( pif1.frame < pif2.frame ) {
			++it1;
		} else {
			++it2;
		}
	}
	return false;
}

/**
 * For a particular surfel, populate the list of neighbouring surfels.
 * A surfel Sn is a neighbour of another surfel S iff:
 * S is represented in a frame F by point P AND
 * Sn is represented in frame F by point Pn AND
 * P and Pn are neighbours.
 * The list of neighbours for a surfel is unique, that is, no mater how many frames
 * contain projections of S and Sn which are neighbours, Sn will occur only once in 
 * the ilst of S's neighbours.
 * @param surfels The list of all surfels.
 * @param neighbours 
 */
void 
populate_neighbours(std::vector<Surfel>& surfels) {
	using namespace std;

	cout << "Populating neighbour : " << flush;

	int count = 0;
	int target = surfels.size();

	for( int i=0; i<surfels.size()-1; ++i ) {
		cout << "\rPopulating neighbour : " << ++count << " of " << target << flush;
		for( int j=i+1; j<surfels.size(); ++j) {
			if( are_neighbours(surfels.at(i), surfels.at(j)) ) {
				surfels.at(i).neighbouring_surfels.push_back(j);
				surfels.at(j).neighbouring_surfels.push_back(i);
			}
		}
	}
	cout << endl;
}

/**
 * For each entry in the correspondence group, we need to construct a 
 * FrameData object which tells us the frame and pixel affected as
 * well as the required transform.
 */
void
populate_frame_data( const std::vector<PixelInFrame>&	correspondence_group,
					 const std::vector<DepthMap>& 		depth_maps,
					 std::vector<FrameData>& 			frame_data) {
	using namespace std;
	using namespace Eigen;

	Vector3f y{ 0.0, 1.0, 0.0};
	for( auto c : correspondence_group) {
		vector<float> n = depth_maps.at(c.frame).get_normals().at(c.y).at(c.x);
		Vector3f target_normal{n.at(0), n.at(1), n.at(2)};
		float depth = depth_maps.at(c.frame).depth_at(c.y, c.x);
		FrameData fd{c, depth, vector_to_vector_rotation( y, target_normal ), target_normal};
		frame_data.push_back( fd );
	}
}

/**
 * Produce a string representing the path to a file in a dirctory
 * @param directory The path of the directory.
 * @param filename The name of the file.
 */
inline std::string file_in_directory(const std::string& directory, const std::string& filename ) {
    // FIXME: Replace this evilness with something more robust and cross platform.
    std::string full_path = directory;
    full_path .append("/").append(filename);
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
load_depth_maps(const std::string& source_directory, std::vector<DepthMap>& depth_maps) {
	using namespace std;
	cout << "Reading depth maps ..." << flush;


	vector<string> files = get_depth_files_in_directory(source_directory);
  	if( files.size() == 0 ) {
  		throw runtime_error( "No depth images found in " + source_directory);
  	}

  	depth_maps.clear();
  	int count = 0;
  	int target = files.size();
  	for( auto file_name : files ) {
  		cout << " \r" << ++count << " of " << target << flush;
  		DepthMap dm{file_name};
  		depth_maps.push_back(dm);
  	}
	cout << endl;
}


/**
 * Preprocess depth maps to 
 * - remove noise
 * - remove dubious data (ie edge points)
 */
void 
preprocess_depth_maps(std::vector<DepthMap>& depth_maps) {
	using namespace std;
	cout << "Preprocessing depth maps : " << flush;

	const float TS = 8.0f;
	const float TL = 3.0f;
	int count = 0;
	int target = depth_maps.size();

	for( auto dm : depth_maps ) {
		cout << "\rPreprocessing depth maps : " << ++count << " of " << target << flush;
		dm.cull_unreliable_depths(TS, TL);
		dm.get_normals();
	}
	cout << endl;
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

	cout << "Computing correspondences..." << flush;


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
	for (auto it = vertex_to_frame_pixel.begin(); it != vertex_to_frame_pixel.end(); ) {
		correspondence.clear();
		unsigned int vertex_id = it->first;
		do {
			correspondence.push_back(it->second);
			++it;
		} while( (it != vertex_to_frame_pixel.end()) && (vertex_id == it->first));
		correspondences.push_back(correspondence);
	}
	cout << endl;
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
	cout << "Generating surfels : " << flush;

	surfels.clear();

	// One per correpondence
	// For each one, add point, depth, normal
	map<PixelInFrame, size_t> pixel_in_frame_to_surfel;

	int count = 0;
	int target = correspondences.size();
	for( auto const & correspondence_group : correspondences ) {
		Surfel surfel;

		cout << "\rGenerating surfels : " << ++count << " of " << target << flush;

		surfel.id = surfels.size();
		populate_frame_data(correspondence_group, depth_maps, surfel.frame_data);
		surfels.push_back( surfel );
		for( auto c : correspondence_group ) {
			pixel_in_frame_to_surfel.insert( make_pair<>(c, surfel.id ));
		}
	}
	cout << endl;
	populate_neighbours(surfels);
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

  cout << "Loading from directory " << file_or_directory << "..." << endl;

  vector<DepthMap> depth_maps;
  load_depth_maps(dir, depth_maps);

  preprocess_depth_maps(depth_maps);

  vector<vector<PixelInFrame>> correspondences;
  compute_correspondences(dir, depth_maps, correspondences);

  generate_surfels(depth_maps, correspondences, surfels);

  cout << " done." << endl;
  // cout << "Processing " << files.size() << " depth images..." << flush;
  // process_depth_files(files, surfels, point_clouds)
}