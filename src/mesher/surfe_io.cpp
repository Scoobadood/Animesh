#include "surfel_io.h"
#include <iostream>

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

    cout << "Loading from file " << file_name << "..." << flush;

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
            for(float & mIdx : m) {
                mIdx = read_float(file);
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