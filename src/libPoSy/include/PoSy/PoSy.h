#include <vector>
#include <Eigen/Core>

/**
 * Given a point in space, a normal, tangent vector and rho (lattice spacing) compute the 8 nearest vertices on the lattice
 */
std::vector<Eigen::Vector3f> compute_lattice_neighbours(
        const Eigen::Vector3f &lattice_origin,
        const Eigen::Vector3f &point,
        const Eigen::Vector3f &tangent,
        const Eigen::Vector3f &orth_tangent,
        float rho);

/**
* Given a regular grid with basis vectors tangent, o_prime,
* a lattice position lattice_point and vertex, a position possibly not on the lattice,
* The following operation rounds vertex to the nearest lattice point.
*/
Eigen::Vector3f
round_4(const Eigen::Vector3f &normal,
        const Eigen::Vector3f &tangent,
        const Eigen::Vector3f &lattice_point,
        const Eigen::Vector3f &vertex,
        float rho);

Eigen::Vector3f
floor_4(const Eigen::Vector3f &n,
        const Eigen::Vector3f &o,
        const Eigen::Vector3f &p,
        const Eigen::Vector3f &q,
        float rho);

/**
 * @return The 3D point offset from p by t_ij lots of o and o'.
 */
Eigen::Vector3f
translate_4(const Eigen::Vector3f &p,
            const Eigen::Vector3f &n,
            const Eigen::Vector3f &o,
            const Eigen::Vector2i &t_ij,
            float rho);

/**
 * Compute the midpoint between two points on planes defined by normal vectors
 * n_i and n_j. The midpoint is forced to lie on the line of intersection of the planes.
 * @param v_i Point on first plane.
 * @param n_i Normal of first plane.
 * @param v_j Point on second plane.
 * @param n_j Nrmal of second plane.
 * @return
 */
Eigen::Vector3f
compute_qij(
        const Eigen::Vector3f &v_i,
        const Eigen::Vector3f &n_i,
        const Eigen::Vector3f &v_j,
        const Eigen::Vector3f &n_j
);

/**
 */
void
compute_t_ij(
        const Eigen::Vector3f &p_i,
        const Eigen::Vector3f &n_i,
        const Eigen::Vector3f &o_i,
        const std::vector<Eigen::Vector2i> &Qij,

        const Eigen::Vector3f &p_j,
        const Eigen::Vector3f &n_j,
        const Eigen::Vector3f &o_j,
        const std::vector<Eigen::Vector2i> &Qji,

        Eigen::Vector2i &t_ij,
        Eigen::Vector2i &t_ji,

        float rho);

Eigen::Vector3f rotate_pj_into_n(
        const Eigen::Vector3f &n_i,
        const Eigen::Vector3f &n_j,
        const Eigen::Vector3f &p_j,
        const Eigen::Vector3f &q_ij);

std::pair<Eigen::Vector3f, Eigen::Vector3f>
find_closest_points(const std::vector<Eigen::Vector3f> &points_a, const std::vector<Eigen::Vector3f> &points_b);

std::pair<Eigen::Vector2i, Eigen::Vector2i>
best_posy_offset(const Eigen::Vector3f &this_vertex,
                 const Eigen::Vector3f &this_tangent,
                 const Eigen::Vector3f &this_normal,
                 const Eigen::Vector2f &this_offset,
                 const Eigen::Vector3f &that_vertex,
                 const Eigen::Vector3f &that_tangent,
                 const Eigen::Vector3f &that_normal,
                 const Eigen::Vector2f &that_offset,
                 float rho
);

std::pair<Eigen::Vector3f, Eigen::Vector3f>
best_posy_offset_vertices(const Eigen::Vector3f &this_vertex,
                          const Eigen::Vector3f &this_tangent,
                          const Eigen::Vector3f &this_normal,
                          const Eigen::Vector2f &this_offset,
                          Eigen::Vector2i& t_ij,
                          const Eigen::Vector3f &that_vertex,
                          const Eigen::Vector3f &that_tangent,
                          const Eigen::Vector3f &that_normal,
                          const Eigen::Vector2f &that_offset,
                          Eigen::Vector2i& t_ji,
                          float rho
);