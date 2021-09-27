//
// Created by Dave Durbin on 2019-07-06.
//

#include "io_utils.h"
#include <fstream>
#include <Eigen/Core>

/**
 * Write an unisgned int
 */
void
write_unsigned_int( std::ofstream& file, unsigned int value ) {
    file.write( (const char *)&value, sizeof( unsigned int ) );
}

/**
 * Write an int (32 bits)
 */
void
write_int( std::ofstream& file, int value ) {
    file.write( (const char *)&value, sizeof( int ) );
}

/**
 * Write an unsigned short (16 bits)
 */
void
write_unsigned_short( std::ofstream& file, unsigned short value ) {
    file.write( (const char *)&value, sizeof( unsigned short ) );
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
write_vector_3f( std::ofstream& file, const Eigen::Vector3f& vector ) {
    write_float(file, vector.x());
    write_float(file, vector.y());
    write_float(file, vector.z());
}

/*
 * Write a vector
 */
void
write_vector_2f( std::ofstream& file, const Eigen::Vector2f& vector ) {
    write_float(file, vector.x());
    write_float(file, vector.y());
}
/*
 * Write a 2D int vector
 */
void
write_vector_2i( std::ofstream& file, const Eigen::Vector2i& vector ) {
    write_int(file, vector.x());
    write_int(file, vector.y());
}

int
read_int( std::ifstream& file ) {
    int i;
    file.read( (char *)&i, sizeof(i) );
    return i;
}

unsigned int
read_unsigned_int( std::ifstream& file ) {
    unsigned int i;
    file.read( (char *)&i, sizeof(i) );
    return i;
}

unsigned short
read_unsigned_short( std::ifstream& file ) {
    unsigned short i;
    file.read( (char *)&i, sizeof(i) );
    return i;
}

std::string
read_string( std::ifstream& file ) {
    std::string str;
    size_t size;
    file.read((char *)&size, sizeof(size));
    str.resize(size);
    file.read(&str[0], size);
    return str;
}

void
write_string(std::ofstream& file, const std::string& value ) {
    size_t size=value.size();
    file.write((char *) & size,sizeof(size));
    file.write(value.c_str(),size);
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


Eigen::Vector2i
read_vector_2i( std::ifstream& file ) {
    int x, y;
    file.read( (char *)&x, sizeof(int) );
    file.read( (char *)&y, sizeof(int) );
    return Eigen::Vector2i{x, y};
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

template<typename Out>
void split(const std::string &s, char delim, Out result) {
  std::stringstream ss(s);
  std::string item;
  while (std::getline(ss, item, delim)) {
    *(result++) = item;
  }
}

std::vector<std::string>
split(const std::string &s, char delim) {
  std::vector<std::string> elems;
  split(s, delim, std::back_inserter(elems));
  return elems;
}
