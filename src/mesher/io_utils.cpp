//
// Created by Dave Durbin on 2019-07-06.
//

#include "io_utils.h"

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
write_size_t( std::ofstream& file, size_t value ) {
    file.write( (const char *)&value, sizeof( size_t ) );
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

unsigned int
read_unsigned_int( std::ifstream& file ) {
    unsigned int i;
    file.read( (char *)&i, sizeof(i) );
    return i;
}

size_t
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
