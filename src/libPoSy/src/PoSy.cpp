
#include <PoSy/PoSy.h>
#include <Geom/Geom.h>
#include <Eigen/Core>
#include <Eigen/Geometry>
#include <vector>
#include <tuple>

/**
 * Given two sets of 3D positions, return the indices and values of the closest two along with the squared distance between them.
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
 * Given a point in space which represents a lattice intersection
 * A normal to the tangent plane
 * A tangent vector and rho (lattice spacing) compute the 8 nearest vertices on the lattice
 */
std::vector<Eigen::Vector3f> compute_local_lattice_vertices(
        const Eigen::Vector3f& lattice_vertex_position,
        const Eigen::Vector3f& u_tangent,
        const Eigen::Vector3f& v_tangent,
        const float rho ) {
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
    const auto l1 = compute_local_lattice_vertices(p1, n1, o1, rho);
    const auto l2 = compute_local_lattice_vertices(p2, n2, o2, rho);
    const auto tuple = closest_points(l1, l2);
    const auto delta = (std::get<3>(tuple) - std::get<2>(tuple));

    const auto u_delta = o1.dot(delta) * o1;
    const auto v = n1.cross(o1);
    const auto v_delta = v.dot(delta) * v;

    const auto new_position =  p1 + ((u_delta + v_delta)  * ( weight1/ (weight1 + weight2)));
    return new_position;
}
