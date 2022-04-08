
#include <PoSy/PoSy.h>
#include <Eigen/Core>
#include <tuple>
#include <spdlog/spdlog.h>

Eigen::Vector3f
position_floor(const Eigen::Vector3f &anchor, // Origin
               const Eigen::Vector3f &tangent, // Rep tan
               const Eigen::Vector3f &orth_tangent, // Rep tan
               const Eigen::Vector3f &point, // Point
               float scale) {
  using namespace Eigen;
  auto inv_scale = 1.0f / scale;

  auto d = point - anchor;
  // Computes the 'bottom left' lattice point closest to point
  return anchor +
      tangent * std::floor(tangent.dot(d) * inv_scale) * scale +
      orth_tangent * std::floor(orth_tangent.dot(d) * inv_scale) * scale;
}

Eigen::Vector2i
position_floor_index(const Eigen::Vector3f &anchor,
                     const Eigen::Vector3f &tangent,
                     const Eigen::Vector3f &orth_tangent,
                     const Eigen::Vector3f &point,
                     float scale) {
  using namespace Eigen;

  auto inv_scale = 1.0f / scale;
  auto delta = point - anchor;
  return {
      (int) std::floor(tangent.dot(delta) * inv_scale),
      (int) std::floor(orth_tangent.dot(delta) * inv_scale)
  };
}

/**
 *
 * @param anchor Origin?
 * @param tangent Rep tangent
 * @param point Point to round?
 * @param inv_scale
 * @return
 */
Eigen::Vector3f position_round(const Eigen::Vector3f &anchor,
                               const Eigen::Vector3f &tangent,
                               const Eigen::Vector3f &orth_tangent,
                               const Eigen::Vector3f &point,
                               float scale) {
  using namespace Eigen;
  auto inv_scale = 1.0f / scale;

  // d is the direction vector from anchor to point
  Vector3f d = point - anchor;

  return anchor +
      tangent * std::round(tangent.dot(d) * inv_scale) * scale +
      orth_tangent * std::round(orth_tangent.dot(d) * inv_scale) * scale;
}

/**
 * Compute a position qij that minimizes the distance to vertices vi and vj while being located
 * in their respective tangent planes.
 * So:
 * Minimize |x - vertex|^2 + |x - other_vertex|^2, where
 * dot(normal, x) == dot(normal, vertex)
 * dot(other_normal, x) == dot(other_normal, other_vertex)
 *
 * -> Lagrange multipliers, set derivative = 0
 *  Use first 3 equalities to write x in terms of
 *  lambda_1 and lambda_2. Substitute that into the last
 *  two equations and solve for the lambdas. Finally,
 *  add a small epsilon term to avoid issues when other_normal=n2.
 * @param vertex Point on first plane.
 * @param normal Normal of first plane.
 * @param other_vertex Point on second plane.
 * @param other_normal Nrmal of second plane.
 * @return
 */
Eigen::Vector3f
compute_qij(
    const Eigen::Vector3f &vertex,
    const Eigen::Vector3f &normal,
    const Eigen::Vector3f &other_vertex,
    const Eigen::Vector3f &other_normal
) {
  auto n0p0 = normal.dot(vertex), n0p1 = normal.dot(other_vertex),
      n1p0 = other_normal.dot(vertex), n1p1 = other_normal.dot(other_vertex),
      n0n1 = normal.dot(other_normal),
      denom = 1.0f / (1.0f - n0n1 * n0n1 + 1e-4f),
      lambda_0 = 2.0f * (n0p1 - n0p0 - n0n1 * (n1p0 - n1p1)) * denom,
      lambda_1 = 2.0f * (n1p0 - n1p1 - n0n1 * (n0p1 - n0p0)) * denom;

  return 0.5f * (vertex + other_vertex) - 0.25f * (normal * lambda_0 + other_normal * lambda_1);
}

std::pair<Eigen::Vector2i, Eigen::Vector2i>
compute_tij_tji(
    const Eigen::Vector3f &vertex,
    const Eigen::Vector3f &normal,
    const Eigen::Vector3f &tangent,
    const Eigen::Vector3f &orth_tangent,
    const Eigen::Vector3f &lattice_point,
    const Eigen::Vector3f &other_vertex,
    const Eigen::Vector3f &other_normal,
    const Eigen::Vector3f &other_tangent,
    const Eigen::Vector3f &other_orth_tangent,
    const Eigen::Vector3f &other_lattice_point,
    float scale, float *error) {
  using namespace Eigen;

  auto middle = compute_qij(vertex, normal, other_vertex, other_normal);
  Vector2i t_ij_to_middle = position_floor_index(lattice_point, tangent, orth_tangent, middle, scale);
  Vector2i t_ji_to_middle = position_floor_index(other_lattice_point, other_tangent, other_orth_tangent, middle, scale);

  auto best_cost = std::numeric_limits<float>::infinity();
  int best_i = -1, best_j = -1;

  for (int i = 0; i < 4; ++i) {
    Vector3f o0t = lattice_point +
        (tangent * ((i & 1) + t_ij_to_middle[0]) +
            orth_tangent * (((i & 2) >> 1) + t_ij_to_middle[1])) * scale;
    for (int j = 0; j < 4; ++j) {
      Vector3f o1t = other_lattice_point
          + (other_tangent * ((j & 1) + t_ji_to_middle[0]) + other_orth_tangent * (((j & 2) >> 1) + t_ji_to_middle[1]))
              * scale;
      float cost = (o0t - o1t).squaredNorm();

      if (cost < best_cost) {
        best_i = i;
        best_j = j;
        best_cost = cost;
      }
    }
  }
  if (error)
    *error = best_cost;

  return std::make_pair(
      Vector2i((best_i & 1) + t_ij_to_middle[0], ((best_i & 2) >> 1) + t_ij_to_middle[1]),
      Vector2i((best_j & 1) + t_ji_to_middle[0], ((best_j & 2) >> 1) + t_ji_to_middle[1]));
}

std::pair<Eigen::Vector3f, Eigen::Vector3f>
compute_closest_lattice_points(
    const Eigen::Vector3f &vertex, // Vertex 0
    const Eigen::Vector3f &normal, // Normal in first plane
    const Eigen::Vector3f &tangent, // Tan in first plane
    const Eigen::Vector3f &orth_tangent, // Tan in first plane
    const Eigen::Vector3f &lattice_vertex, // Origin in first plane
    const Eigen::Vector3f &other_vertex, // Vertex 1
    const Eigen::Vector3f &other_normal, // Normal in second plane
    const Eigen::Vector3f &other_tangent, // Tan in second plane
    const Eigen::Vector3f &othert_orth_tangent, // Tan in second plane
    const Eigen::Vector3f &other_lattice_vertex, // Origin in second plane
    float scale) {
  using namespace Eigen;

  // Compute q_ij; the midpoint of vertex, other_vertex
  auto middle = compute_qij(vertex, normal, other_vertex, other_normal);

  // Origin, reptan, normal, point
  auto this_base_lattice_point = position_floor(lattice_vertex, tangent, orth_tangent, middle, scale);
  auto that_base_lattice_point = position_floor(other_lattice_vertex, other_tangent, othert_orth_tangent, middle, scale);

  auto best_cost = std::numeric_limits<float>::infinity();
  int best_i = -1, best_j = -1;

  for (int i = 0; i < 4; ++i) {
    // Derive ,o0t (test)  sequentiall, the other bounds of 1_ij in first plane
    Vector3f o0t = this_base_lattice_point + (tangent * (i & 1) + orth_tangent * ((i & 2) >> 1)) * scale;
    for (int j = 0; j < 4; ++j) {
      Vector3f o1t = that_base_lattice_point + (other_tangent * (j & 1) + othert_orth_tangent * ((j & 2) >> 1)) * scale;
      auto cost = (o0t - o1t).squaredNorm();

      if (cost < best_cost) {
        best_i = i;
        best_j = j;
        best_cost = cost;
      }
    }
  }

  // best_i and best_j are the closest vertices surrounding q_ij
  // we return the pair of closest points on the lattice according to both origins
  return std::make_pair(
      this_base_lattice_point + (tangent * (best_i & 1) + orth_tangent * ((best_i & 2) >> 1)) * scale,
      that_base_lattice_point + (other_tangent * (best_j & 1) + othert_orth_tangent * ((best_j & 2) >> 1)) * scale);
}