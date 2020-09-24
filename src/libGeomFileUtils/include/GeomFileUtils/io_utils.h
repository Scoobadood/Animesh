//
// Created by Dave Durbin on 2019-07-06.
//

#pragma once

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
write_vector_3f( std::ofstream& file, const Eigen::Vector3f& vector );

/*
 * Write a vector
 */
void
write_vector_2f( std::ofstream& file, const Eigen::Vector2f& vector );

unsigned int
read_unsigned_int( std::ifstream& file );

size_t
read_size_t( std::ifstream& file );

float
read_float( std::ifstream& file );


Eigen::Vector2f
read_vector_2f( std::ifstream& file );

Eigen::Vector3f
read_vector_3f( std::ifstream& file );


void
write_string(std::ofstream& file, const std::string& value );

std::string
read_string( std::ifstream& file );