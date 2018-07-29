#pragma once

#include <Geom/geom.h>

#include <Eigen/Core>

/**
 * @param targetVector The vector we're trying to match
 * @param targetK The value of K for the target vector which should be locked
 * @param normal The normal about which to rotate the sourceVector
 * @param sourceVector the vector to be matched
 * @return the best fitting vector (i.e. best multiple of PI/2 + angle)
 * 
 */
Eigen::Vector3f best_rosy_vector_for( const Eigen::Vector3f& target_vector, 
									  const Eigen::Vector3f& target_normal, 
									  const Eigen::Vector3f& source_vector, 
									  const Eigen::Vector3f& source_normal,
									  int& k_ij );

/**
 * @param targetVector The vector we're trying to match
 * @param normal The normal about which to rotate the sourceVector
 * @param sourceVector the vector to be matched
 * @return the best fitting vector (i.e. best multiple of PI/2 + angle)
 * 
 */
Eigen::Vector3f best_dp_rosy_vector_for( const Eigen::Vector3f& target_vector, 
										 const Eigen::Vector3f& target_normal, 
										 const Eigen::Vector3f& source_vector, 
										 const Eigen::Vector3f& source_normal,
										 int& k_ij );

/**
 * @param targetVector The vector we're trying to match
 * @param normal The normal about which to rotate the sourceVector
 * @param sourceVector the vector to be matched
 * @return the best fitting vector (i.e. best multiple of PI/2 + angle)
 * 
 */
std::pair<Eigen::Vector3f, Eigen::Vector3f>  best_rosy_vector_pair( const Eigen::Vector3f& target_vector, const Eigen::Vector3f& target_normal, 
						  	  									  const Eigen::Vector3f& source_vector, const Eigen::Vector3f& source_normal);

/**
 * Combine two tangent vectors with weighting
 * @param v1 The first vector
 * @param v2 The second vector
 * @param n1 The first normal 
 * @param n2 The second normal
 * @param w1 Weighting for the first vector
 * @param w2 Weighting for the second vector
 */
Eigen::Vector3f average_rosy_vectors( const Eigen::Vector3f& v1, 
									 const Eigen::Vector3f& n1,
									 float w1, 
									 const Eigen::Vector3f& v2, 
									 const Eigen::Vector3f& n2,
									 float w2);
