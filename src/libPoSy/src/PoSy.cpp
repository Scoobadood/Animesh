
#include <PoSy/PoSy.h>
#include <Geom/Geom.h>
#include <Eigen/Core>
#include <Eigen/Geometry>
#include <vector>
#include <tuple>

/**
 * Given two sets of 3D positions, return the values of the closest two along with the squared distance between them.
 * This is O(n^2) so only really suitable for very small numbers of points. We use it for two sets of 9 points.
 * @param points_a
 * @param points_b
 * @return a tuple consisting of the two closest points and the suqared distance betwen them.
 */
std::tuple<Eigen::Vector3f, Eigen::Vector3f, float>
closest_points(const std::vector<Eigen::Vector3f> &points_a,
               const std::vector<Eigen::Vector3f> &points_b) {
    using namespace std;
    using namespace Eigen;

    assert(!points_a.empty());
    assert(!points_b.empty());

    float min_dist_squared = std::numeric_limits<float>::max();
    size_t best_idx_a = 0;
    size_t best_idx_b = 0;
    for (auto idx_a = 0; idx_a < points_a.size(); ++idx_a) {
        for (auto idx_b = 0; idx_b < points_b.size(); ++idx_b) {
            const auto diff = points_a.at(idx_a) - points_b.at(idx_b);
            const auto dist = diff.x() * diff.x() + diff.y() * diff.y() + diff.z() * diff.z();
            if (dist < min_dist_squared) {
                best_idx_a = idx_a;
                best_idx_b = idx_b;
                min_dist_squared = dist;
            }
        }
    }
    return make_tuple(points_a.at(best_idx_a), points_b.at(best_idx_b), min_dist_squared);
}


/**
 * Given a point in space which represents a lattice intersection
 * A normal to the tangent plane
 * A tangent vector and rho (lattice spacing) compute the 8 nearest vertices on the lattice
 */
std::vector<Eigen::Vector3f> compute_surrounding_vertices(
        const Eigen::Vector3f &lattice_vertex_position,
        const Eigen::Vector3f &u_tangent,
        const Eigen::Vector3f &v_tangent,
        const float rho) {
    using namespace Eigen;
    using namespace std;

    vector<Vector3f> lattice_vertices;
    lattice_vertices.push_back(lattice_vertex_position);

    // 'right'
    lattice_vertices.emplace_back(lattice_vertex_position + rho * u_tangent);
    // 'down right'
    lattice_vertices.emplace_back(lattice_vertex_position + rho * u_tangent + rho * v_tangent);
    // 'down'
    lattice_vertices.emplace_back(lattice_vertex_position + rho * v_tangent);
    // 'down left'
    lattice_vertices.emplace_back(lattice_vertex_position - rho * u_tangent + rho * v_tangent);
    // 'left'
    lattice_vertices.emplace_back(lattice_vertex_position - rho * u_tangent);
    // 'up left'
    lattice_vertices.emplace_back(lattice_vertex_position - rho * u_tangent - rho * v_tangent);
    // 'up'
    lattice_vertices.emplace_back(lattice_vertex_position - rho * v_tangent);
    // 'up right'
    lattice_vertices.emplace_back(lattice_vertex_position + rho * u_tangent - rho * v_tangent);

    return lattice_vertices;
}


/**
 * Compute the new update for the given node
 */
Eigen::Vector3f
average_posy_vectors(const Eigen::Vector3f &p1,
                     const Eigen::Vector3f &o1,
                     const Eigen::Vector3f &n1,
                     float weight1,
                     const Eigen::Vector3f &p2,
                     const Eigen::Vector3f &o2,
                     const Eigen::Vector3f &n2,
                     float weight2,
                     float rho
) {
    const auto l1 = compute_surrounding_vertices(p1, n1, o1, rho);
    const auto l2 = compute_surrounding_vertices(p2, n2, o2, rho);
    const auto tuple = closest_points(l1, l2);
    const auto delta = (std::get<1>(tuple) - std::get<0>(tuple));

    const auto u_delta = o1.dot(delta) * o1;
    const auto v = n1.cross(o1);
    const auto v_delta = v.dot(delta) * v;

    const auto new_position = p1 + ((u_delta + v_delta) * (weight1 / (weight1 + weight2)));
    return new_position;
}


/**
 * Compute the error in the source surfel's estimate of the lattice position
 * as estimated by the target surfel. The lattice is assumed to be
 * aligned with source_u and source_v
 * @param source_position Actual position of the source surfel.
 * @param source_u Tangent plane vector for source surfel.
 * @param source_v Tangent plane vector for source surfel perpendicular to source_u.
 * @param source_uv_offset Expected displacement of nearest lattice vertex from source.
 * @param target_position Actual position of the target surfel.
 * @param target_u Tangent plane vector for target surfel.
 * @param target_v Tangent plane vector for target surfel perpendicular to target_u.
 * @param target_uv_offset Expected displacement of nearest lattice vertex from target.
 * @param rho Lattice spacing
 * @return The correction to be applied to the source_uv_offset to make it agree with the target.
 */
Eigen::Vector2f compute_source_uv_correction(
        const Eigen::Vector3f &source_position,
        const Eigen::Vector3f &source_u,
        const Eigen::Vector3f &source_v,
        const Eigen::Vector2f &source_uv_offset,

        const Eigen::Vector3f &target_position,
        const Eigen::Vector3f &target_u,
        const Eigen::Vector3f &target_v,
        const Eigen::Vector2f &target_uv_offset,

        float rho
) {
    using namespace std;
    using namespace Eigen;

    const auto source_nearest_lattice_vertex =
            source_position + (source_u * source_uv_offset.x()) + (source_v * source_uv_offset.y());
    const auto target_nearest_lattice_vertex =
            target_position + (target_u * target_uv_offset.x()) + (target_v * target_uv_offset.y());

    const auto l1 = compute_surrounding_vertices(source_nearest_lattice_vertex, source_u, source_v, rho);
    const auto l2 = compute_surrounding_vertices(target_nearest_lattice_vertex, target_u, target_v, rho);

    // following returns (best_idx_a, best_idx_b, points_a.at(best_idx_a), points_b.at(best_idx_b), min_dist_squared)
    const auto tuple = closest_points(l1, l2);

    // Actual distance between lattice points in source_u and source_v dimensions
    const auto delta_xyz = get<1>(tuple) - get<0>(tuple);
    const auto delta_u = delta_xyz.dot(source_u);
    const auto delta_v = delta_xyz.dot(source_v);

    // We would hope that these are _exact_ multiples of rho. The distortion is the amount by which they differ
    // from an exact multiple of rho.
    auto error_u = std::remainderf(delta_u, rho);
    auto error_v = std::remainderf(delta_v, rho);

    // This is the correction
    return {-error_u, -error_v};
}
