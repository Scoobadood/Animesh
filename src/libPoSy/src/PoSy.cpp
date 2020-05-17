#include <PoSy/PoSy.h>
#include <Eigen/Core>
#include <Eigen/Geometry>
#include <vector>
#include <tuple>

/**
 * Given two sets of 3D positions, return the indices and values of the closest two along with the distance between them.
 */
std::tuple<size_t, size_t, Eigen::Vector3f, Eigen::Vector3f, float>
closest_points(const std::vector<Eigen::Vector3f>& points_a, const std::vector<Eigen::Vector3f>& points_b ) {
    using namespace std;
    using namespace Eigen;

    assert( !points_a.empty());
    assert( !points_b.empty());

    float min_dist_squared = std::numeric_limits<float>::max();
    size_t best_idx_a = 0;
    size_t best_idx_b = 0;
    for (unsigned int idx_a = 0; idx_a < points_a.size(); ++idx_a) {
        for (unsigned int idx_b = 0; idx_b < points_b.size(); ++idx_b) {
            const auto diff = points_a.at(idx_a) - points_b.at(idx_b);
            const auto d2 = diff.x() * diff.x() + diff.y() * diff.y() + diff.z() * diff.z();
            if (d2 < min_dist_squared) {
                best_idx_a = idx_a;
                best_idx_b = idx_b;
                min_dist_squared = d2;
            }
        }
    }
    return make_tuple(best_idx_a, best_idx_b, points_a.at(best_idx_a), points_b.at(best_idx_b), min_dist_squared);
}


/**
 * Given a point in space, a normal, tangent vector and rho (lattice spacing) compute the 8 nearest vertices on the lattice
 */
std::vector<Eigen::Vector3f> compute_local_lattice_vertices(
        const Eigen::Vector3f& position,
        const Eigen::Vector3f& unit_normal,
        const Eigen::Vector3f& tangent,
        const float rho ) {
    using namespace Eigen;
    using namespace std;

    vector<Vector3f> lattice_vertices;
    lattice_vertices.push_back(position);

    const auto alt_tangent = unit_normal.cross(tangent);
    // 'right'
    lattice_vertices.emplace_back(position + rho * tangent);
    // 'down right'
    lattice_vertices.emplace_back(position + rho * tangent + rho * alt_tangent);
    // 'down'
    lattice_vertices.emplace_back(position + rho * alt_tangent);
    // 'down left'
    lattice_vertices.emplace_back(position - rho * tangent + rho * alt_tangent);
    // 'left'
    lattice_vertices.emplace_back(position - rho * tangent);
    // 'up left'
    lattice_vertices.emplace_back(position - rho * tangent - rho * alt_tangent);
    // 'up'
    lattice_vertices.emplace_back(position - rho * alt_tangent);
    // 'up right'
    lattice_vertices.emplace_back(position + rho * tangent - rho * alt_tangent);

    return lattice_vertices;
}


/**
 * Compute the new update for the given node
 */
Eigen::Vector3f
average_posy_vectors(const Eigen::Vector3f &p1,
                     const Eigen::Vector3f &o1,
                     const Eigen::Vector3f &n1,
                     const Eigen::Vector3f &p2,
                     const Eigen::Vector3f &o2,
                     const Eigen::Vector3f &n2,
                     const float rho,
                     const float weight
) {
    const auto l1 = compute_local_lattice_vertices(p1, n1, o1, rho);
    const auto l2 = compute_local_lattice_vertices(p2, n2, o2, rho);
    const auto tuple = closest_points(l1, l2);
    const auto delta = (std::get<2>(tuple) - std::get<3>(tuple));
    return p1 + weight * delta;
}
