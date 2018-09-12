#pragma once 

#include <string>
#include <Eigen/Core>

/**
 * Check that a vector is of unit length or throw
 */
void checkUnitLength( const std::string& vector_name, const Eigen::Vector3f& vector ); 

/**
 * Check that two vectors are perpendicular or throw
 */
void checkPerpendicular( const std::string& vec1_name, const Eigen::Vector3f& vec1, const std::string& vec2_name, const Eigen::Vector3f& vec2 );


void checkRotationMatrix( const std::string& matrix_name, const Eigen::Matrix3f& R);

void checkNotZeroVector(const std::string& vector_name, const Eigen::Vector3f& v);
