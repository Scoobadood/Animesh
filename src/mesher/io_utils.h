//
// Created by Dave Durbin on 2019-07-06.
//

#ifndef ANIMESH_IO_UTILS_H
#define ANIMESH_IO_UTILS_H

#include <fstream>
#include <Eigen/Core>

/**
 * Write an unisgned int
 */
void
write_unsigned_int( std::ofstream& file, unsigned int value );

/**
 * Write a float
 */
void
write_float( std::ofstream& file, float value );

/**
 * Write an unisgned int
 */
void
write_size_t( std::ofstream& file, size_t value );

/*
 * Write a vector
 */
void
write_vector_3f( std::ofstream& file, Eigen::Vector3f vector );

unsigned int
read_unsigned_int( std::ifstream& file );

size_t
read_size_t( std::ifstream& file );

float
read_float( std::ifstream& file );


Eigen::Vector3f
read_vector_3f( std::ifstream& file );


void
write_string(std::ofstream& file, std::string value );

std::string
read_string( std::ifstream& file );
#endif //ANIMESH_IO_UTILS_H
