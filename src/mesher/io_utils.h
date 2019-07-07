//
// Created by Dave Durbin on 2019-07-06.
//

#ifndef ANIMESH_IO_UTILS_H
#define ANIMESH_IO_UTILS_H

#include <fstream>

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

unsigned int
read_unsigned_int( std::ifstream& file );

size_t
read_size_t( std::ifstream& file );

float
read_float( std::ifstream& file );

#endif //ANIMESH_IO_UTILS_H
