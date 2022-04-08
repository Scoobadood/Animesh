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
* a lattice position anchor and point_to_correct, a position possibly not on the lattice,
* The following operation rounds point_to_correct to the nearest lattice point.
*/
Eigen::Vector3f
round_4(const Eigen::Vector3f &normal,
        const Eigen::Vector3f &tangent,
        const Eigen::Vector3f &anchor,
        const Eigen::Vector3f &point_to_correct,
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
 * n0 and n1. The midpoint is forced to lie on the line of intersection of the planes.
 * @param p0 Point on first plane.
 * @param n0 Normal of first plane.
 * @param p1 Point on second plane.
 * @param n1 Nrmal of second plane.
 * @return
 */
Eigen::Vector3f
compute_qij(
    const Eigen::Vector3f &p0,
    const Eigen::Vector3f &n0,
    const Eigen::Vector3f &p1,
    const Eigen::Vector3f &n1
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
                          Eigen::Vector2i &t_ij,
                          const Eigen::Vector3f &that_vertex,
                          const Eigen::Vector3f &that_tangent,
                          const Eigen::Vector3f &that_normal,
                          const Eigen::Vector2f &that_offset,
                          Eigen::Vector2i &t_ji,
                          float rho
);

std::pair<Eigen::Vector2i, Eigen::Vector2i>
compute_tij_pair(
    const Eigen::Vector3f &origin,
    const Eigen::Vector3f &tangent,
    const Eigen::Vector3f &orth_tangent,
    const Eigen::Vector3f &nbr_origin,
    const Eigen::Vector3f &nbr_tangent,
    const Eigen::Vector3f &nbr_orth_tangent,
    const Eigen::Vector3f &midpoint,
    float scale
);

std::pair<Eigen::Vector3f, Eigen::Vector3f>
compute_closest_points(
    const Eigen::Vector3f &lattice1,
    const Eigen::Vector3f &lattice1_u,
    const Eigen::Vector3f &lattice1_v,
    const Eigen::Vector3f &lattice2,
    const Eigen::Vector3f &lattice2_u,
    const Eigen::Vector3f &lattice2_v,
    const Eigen::Vector3f &common_point,
    float scale,
    std::vector<Eigen::Vector3f> &i_vecs,
    std::vector<Eigen::Vector3f> &j_vecs
);

std::pair<Eigen::Vector3f, Eigen::Vector3f>
compute_closest_points(
    const Eigen::Vector3f &lattice_point,
    const Eigen::Vector3f &tangent,
    const Eigen::Vector3f &orth_tangent,
    const Eigen::Vector3f &nbr_lattice_point,
    const Eigen::Vector3f &nbr_tangent,
    const Eigen::Vector3f &nbr_orth_tangent,
    const Eigen::Vector3f &midpoint,
    float scale
);

Eigen::Vector2i
position_floor_index(
    const Eigen::Vector3f &lattice_point,
    const Eigen::Vector3f &tangent,
    const Eigen::Vector3f &orth_tangent,
    const Eigen::Vector3f &midpoint,
    float scale
);

Eigen::Vector3f
position_floor(const Eigen::Vector3f &anchor, // Origin
               const Eigen::Vector3f &tangent, // Rep tan
               const Eigen::Vector3f &normal, // Normal
               const Eigen::Vector3f &point_to_floor, // Point
               float scale);
