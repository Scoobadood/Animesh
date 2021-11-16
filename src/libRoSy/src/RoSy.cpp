#include <iostream>
#include <Eigen/Geometry>
#include <RoSy/RoSy.h>
#include <Geom/Geom.h>

/**
* @param target_vector The vector we're trying to match.
* @param target_normal The normal about which to rotate it.
* @param source_vector The vector we're matching to the target.
* @param source_normal The normal about which to rotate it.
* @return the best fitting vector (i.e. best multiple of PI/2 + angle)
*/
std::pair<Eigen::Vector3f, Eigen::Vector3f>
best_rosy_vector_pair(const Eigen::Vector3f &target_vector, const Eigen::Vector3f &target_normal,
                      const Eigen::Vector3f &source_vector, const Eigen::Vector3f &source_normal) {
    unsigned short source_k = 0, target_k = 0;
    return best_rosy_vector_pair(target_vector, target_normal, target_k, source_vector, source_normal, source_k);
}

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
best_rosy_vector_pair(const Eigen::Vector3f &o_i, const Eigen::Vector3f &n_i, unsigned short &k_ij,
                      const Eigen::Vector3f &o_j, const Eigen::Vector3f &n_j, unsigned short &k_ji) {
    using namespace Eigen;
    using namespace std;

    if (!is_unit_vector(n_j)) {
        throw invalid_argument("Normal must be unit vector");
    }
    if (!is_unit_vector(n_i)) {
        throw invalid_argument("Normal must be unit vector");
    }
    if (is_zero_vector(o_j)) {
        throw invalid_argument("Vector may not be zero length");
    }
    if (is_zero_vector(o_i)) {
        throw invalid_argument("Vector may not be zero length");
    }

  const Vector3f A[2] = {o_i, n_i.cross(o_i)};
  const Vector3f B[2] = {o_j, n_j.cross(o_j)};

  auto best_score = -numeric_limits<float>::infinity();
  k_ij = 0;
  k_ji = 0;

  for (int i = 0; i < 2; ++i) {
    for (int j = 0; j < 2; ++j) {
      auto score = std::abs(A[i].dot(B[j]));
      if (score > best_score) {
        k_ij = i;
        k_ji = j;
        best_score = score;
      }
    }
  }

  const auto dp = A[k_ij].dot(B[k_ji]);
  pair<Vector3f, Vector3f> p = {A[k_ij], B[k_ji] * (dp < 0 ? -1 : 1)};
  if( dp < 0 ) {
    k_ji += 2;
  }
  return p;
}

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
                                     float w2) {
    using namespace Eigen;

    // Find best matching rotation
    std::pair<Vector3f, Vector3f> result = best_rosy_vector_pair(v1, n1, v2, n2);
    Eigen::Vector3f v = (result.first * w1) + (result.second * w2);
    v = project_vector_to_plane(v, n1);
    return v;
}

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
                                     unsigned short &source_k) {
    using namespace Eigen;

    // Find best matching rotation
    std::pair<Vector3f, Vector3f> result = best_rosy_vector_pair(v1, n1, target_k, v2, n2, source_k);
    Eigen::Vector3f v = (result.first * w1) + (result.second * w2);
    v = project_vector_to_plane(v, n1);
    return v;
}
