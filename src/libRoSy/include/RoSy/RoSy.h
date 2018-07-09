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

