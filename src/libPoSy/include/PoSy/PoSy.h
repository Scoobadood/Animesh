#include <vector>
#include <Eigen/Core>

/**
 * Given two sets of 3D positions, return the values of the closest two along with the squared distance between them.
 * This is O(n^2) so only really suitable for very small numbers of points. We use it for two sets of 9 points.
 * @param points_a
 * @param points_b
 * @return a tuple consisting of the two closest points and the suqared distance betwen them.
 */
std::tuple<Eigen::Vector3f, Eigen::Vector3f, float>
closest_points(const std::vector<Eigen::Vector3f> &points_a, const std::vector<Eigen::Vector3f> &points_b);


/**
 * Given a point in space, a normal, tangent vector and rho (lattice spacing) compute the 8 nearest vertices on the lattice
 */
std::vector<Eigen::Vector3f> compute_surrounding_vertices(
        const Eigen::Vector3f &lattice_vertex_position,
        const Eigen::Vector3f &u_tangent,
        const Eigen::Vector3f &v_tangent,
        const float rho);

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
);

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
);
