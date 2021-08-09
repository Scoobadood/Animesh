#pragma once

#include <Geom/Geom.h>

#include <Eigen/Core>


/**
* @param target_vector The vector we're trying to match.
* @param target_normal The normal about which to rotate it.
* @param source_vector The vector we're matching to the target.
* @param source_normal The normal about which to rotate it.
* @return the best fitting vector (i.e. best multiple of PI/2 + angle)
*/
std::pair<Eigen::Vector3f, Eigen::Vector3f>
best_rosy_vector_pair(const Eigen::Vector3f &target_vector,
                      const Eigen::Vector3f &target_normal,
                      const Eigen::Vector3f &source_vector,
                      const Eigen::Vector3f &source_normal);

/**
* @param o_i The vector we're trying to match.
* @param n_i The normal about which to rotate it.
* @param k_ij The number of rotations required for the match (output).
* @param o_j The vector we're matching to the target.
* @param n_j The normal about which to rotate it.
* @param k_ji The number of rotations required for the match (output).
* @return the best fitting vector (i.e. best multiple of PI/2 + angle)
*/
std::pair<Eigen::Vector3f, Eigen::Vector3f>
best_rosy_vector_pair(const Eigen::Vector3f &o_i,
                      const Eigen::Vector3f &n_i, unsigned short &k_ij,
                      const Eigen::Vector3f &o_j,
                      const Eigen::Vector3f &n_j, unsigned short &k_ji);

/**
 * Combine two tangent vectors with weighting
 * @param v1 The first vector
 * @param v2 The second vector
 * @param n1 The first normal
 * @param n2 The second normal
 * @param w1 Weighting for the first vector
 * @param w2 Weighting for the second vector
 */
Eigen::Vector3f average_rosy_vectors(const Eigen::Vector3f &v1,
                                     const Eigen::Vector3f &n1,
                                     float w1,
                                     const Eigen::Vector3f &v2,
                                     const Eigen::Vector3f &n2,
                                     float w2);

/**
 * Combine two tangent vectors with weighting
 * @param v1 The first vector
 * @param v2 The second vector
 * @param n1 The first normal
 * @param n2 The second normal
 * @param w1 Weighting for the first vector
 * @param w2 Weighting for the second vector
 * @param target_k The number of quarter turns of v1 required for the match (output).
 * @param source_k The number of quarter turns of v2 required for the match (output).
*/
Eigen::Vector3f average_rosy_vectors(const Eigen::Vector3f &v1,
                                     const Eigen::Vector3f &n1,
                                     float w1,
                                     const Eigen::Vector3f &v2,
                                     const Eigen::Vector3f &n2,
                                     float w2,
                                     unsigned short &target_k,
                                     unsigned short &source_k);
