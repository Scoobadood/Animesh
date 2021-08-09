
#include <PoSy/PoSy.h>
#include <Geom/Geom.h>
#include <Eigen/Core>
#include <Eigen/Geometry>
#include <vector>
#include <tuple>
#include <spdlog/spdlog.h>

/**
 * Given a point in space, a normal, tangent vector and rho (lattice spacing) compute the 4 nearest vertices on the lattice
 */
std::vector<Eigen::Vector3f> compute_lattice_neighbours(
        const Eigen::Vector3f &lattice_origin,
        const Eigen::Vector3f &point,
        const Eigen::Vector3f &tangent,
        const Eigen::Vector3f &orth_tangent,
        float rho) {
    using namespace std;
    using namespace Eigen;

    const auto inv_rho = 1.0f / rho;
    vector<Vector3f> neighbours;

    const auto diff = point - lattice_origin;

    const auto base = lattice_origin +
                      (tangent * std::floorf(tangent.dot(diff) * inv_rho) * rho) +
                      (orth_tangent * std::floorf(orth_tangent.dot(diff) * inv_rho) * rho);

    neighbours.emplace_back(base);
    neighbours.emplace_back(base + (tangent * rho));
    neighbours.emplace_back(base + (orth_tangent * rho));
    neighbours.emplace_back(base + ((tangent + orth_tangent) * rho));

    return neighbours;
}

/**
 * Given a regular grid with orientation tangent_u,
 * position gridPosition and normal, the following operation rounds a
 * position roundPosition′ to the nearest lattice point.
 * @param gridPosition
 * @param roundPosition
 * @param tangent_u
 * @param tangent_v
 * @param normal
 * @param rho
 * @return
 */
Eigen::Vector3f
round_4(const Eigen::Vector3f &normal,
        const Eigen::Vector3f &tangent,
        const Eigen::Vector3f &lattice_point,
        const Eigen::Vector3f &vertex,
        float rho) {
    const auto inv_rho = 1.0f / rho;

    const auto orth_tangent = normal.cross(tangent);
    const auto diff = (vertex - lattice_point);
    return lattice_point
           + tangent * (std::roundf(diff.dot(tangent) * inv_rho) * rho)
           + orth_tangent * (std::roundf(diff.dot(orth_tangent) * inv_rho) * rho);
}

Eigen::Vector3f
floor_4(const Eigen::Vector3f &n,
        const Eigen::Vector3f &o,
        const Eigen::Vector3f &p,
        const Eigen::Vector3f &q,
        float rho) {
    assert(n.isOrthogonal(o));
    assert(n.isUnitary());
    assert(o.isUnitary());

    const auto inv_rho = 1.0f / rho;

    const auto o_prime = n.cross(o);
    const auto diff = (q - p);
    return p
           + o * (std::floorf(diff.dot(o) * inv_rho) * rho)
           + o_prime * (std::floorf(diff.dot(o_prime) * inv_rho) * rho);
}

/**
 * Compute the integer translation of a position p ∈ R3 and the set of all its possible integer translations.
 *
 * @param p The position
 * @param n The normal at position.
 * @param o The orientation, an arbitrary tangent in the plane.
 * @param t_ij Integral offsets in lattice coordinates.
 * @param rho The spacing of the mesh.
 * @return The new position.
 */
Eigen::Vector3f
translate_4(const Eigen::Vector3f &p,
            const Eigen::Vector3f &n,
            const Eigen::Vector3f &o,
            const Eigen::Vector2i &t_ij,
            float rho) {
    assert(n.isOrthogonal(o));
    assert(n.isUnitary());
    assert(o.isUnitary());
    const auto o_prime = vector_by_rotating_around_n(o, n, 1);
    return p + (rho * (float) t_ij[0] * o) + (rho * (float) t_ij[1] * o_prime);
}

/**
 * Compute a position qij that minimizes the distance to vertices vi and vj while being located
 * in their respective tangent planes.
 * So:
 * Minimize |x - v_i|^2 + |x - v_j|^2, where
 * dot(n_i, x) == dot(n_i, v_i)
 * dot(n_j, x) == dot(n_j, v_j)
 *
 * -> Lagrange multipliers, set derivative = 0
 *  Use first 3 equalities to write x in terms of
 *  lambda_1 and lambda_2. Substitute that into the last
 *  two equations and solve for the lambdas. Finally,
 *  add a small epsilon term to avoid issues when n1=n2.
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
) {
    const double ni_dot_vi = n_i.dot(v_i);
    const double ni_dot_vj = n_i.dot(v_j);
    const double nj_dot_vi = n_j.dot(v_i);
    const double nj_dot_vj = n_j.dot(v_j);
    const double ni_dot_nj = n_i.dot(n_j);


    // If planes are parallel, pick the midpoint
    const double denom = 1.0 - (ni_dot_nj * ni_dot_nj);
    Eigen::Vector3f q = (v_i + v_j) * 0.5;
    if (abs(denom) > 1e-4) {
        const double lambda_i = 2.0 * (ni_dot_vj - ni_dot_vi - ni_dot_nj * (nj_dot_vi - nj_dot_vj)) / denom;
        const double lambda_j = 2.0 * (nj_dot_vi - nj_dot_vj - ni_dot_nj * (ni_dot_vj - ni_dot_vi)) / denom;
        q -= (((lambda_i * n_i) + (lambda_j * n_j)) * 0.25);
    }
    // It's possible that q_ij is not in the plane defined by n_i so correct for that
    return q;
}

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

        float rho) {
    assert(n_i.isOrthogonal(o_i));
    assert(n_i.isUnitary());
    assert(o_i.isUnitary());
    assert(n_j.isOrthogonal(o_j));
    assert(n_j.isUnitary());
    assert(o_j.isUnitary());

    float best_len = MAXFLOAT;
    t_ij = Eigen::Vector2i::Zero();
    t_ji = Eigen::Vector2i::Zero();

    for (auto &t_ij_test : Qij) {
        const auto v1 = translate_4(p_i, n_i, o_i, t_ij_test, rho);
        for (auto &m_ij_test : Qji) {
            const auto v2 = translate_4(p_j, n_j, o_j, m_ij_test, rho);
            const auto diff = v2 - v1;
            const auto len = diff.squaredNorm();
            if (len < best_len) {
                best_len = len;
                t_ij = t_ij_test;
                t_ji = m_ij_test;
            }
        }
    }
}

/**
 *
 */
Eigen::Vector3f rotate_pj_into_n(const Eigen::Vector3f &n_i,
                                 const Eigen::Vector3f &n_j,
                                 const Eigen::Vector3f &p_j,
                                 const Eigen::Vector3f &q_ij) {
    using namespace Eigen;

    // pji := rot(nj × ni, 􏰏(nj,ni)) (pj − qij) + qij,
    const auto axis = n_j.cross(n_i);
    float angle = degrees_angle_between_vectors(n_j, n_i) * M_PI / 180.0f;

    const auto p_ji = rotate_point_through_axis_angle(axis, angle, (p_j - q_ij)) + q_ij;
    return p_ji;
}

std::pair<Eigen::Vector3f, Eigen::Vector3f>
find_closest_points(const std::vector<Eigen::Vector3f> &points_a, const std::vector<Eigen::Vector3f> &points_b) {
    float best_dist = std::numeric_limits<float>::infinity();
    Eigen::Vector3f best_pa, best_pb;
    for (const auto &p_a : points_a) {
        for (const auto &p_b : points_b) {
            const auto squared_dist = (p_b - p_a).squaredNorm();
            if (squared_dist < best_dist) {
                best_pa = p_a;
                best_pb = p_b;
                best_dist = squared_dist;
            }
        }
    }
    return std::make_pair(best_pa, best_pb);
}


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
) {
    using namespace std;
    using namespace Eigen;

    // Compute orth tangents
    const auto this_orth_tangent = this_normal.cross(this_tangent);
    const auto that_orth_tangent = that_normal.cross(that_tangent);

    const auto this_lattice_vertex = this_vertex +
                                     this_offset[0] * this_tangent +
                                     this_offset[1] * this_orth_tangent;
    const auto that_lattice_vertex = that_vertex +
                                     that_offset[0] * that_tangent +
                                     that_offset[1] * that_orth_tangent;

    // Compute q_ij and thus Q_ij and Q_ji
    const auto q = compute_qij(this_vertex, this_normal, that_vertex, that_normal);
    const auto Q_ij = compute_lattice_neighbours(this_lattice_vertex,
                                                 q,
                                                 this_tangent,
                                                 this_orth_tangent,
                                                 rho);
    const auto Q_ji = compute_lattice_neighbours(that_lattice_vertex,
                                                 q,
                                                 that_tangent,
                                                 that_orth_tangent,
                                                 rho);

    // Get the closest points adjacent to q in both tangent planes
    const auto closest_points = find_closest_points(Q_ij, Q_ji);

    // Compute t_ij and t_ji
    const auto dxyz_ij = (closest_points.first - this_lattice_vertex);
    const auto t_ij_0 = dxyz_ij.dot(this_tangent);
    const auto t_ij_1 = dxyz_ij.dot(this_orth_tangent);

    const auto dxyz_ji = (closest_points.second - that_lattice_vertex);
    const auto t_ji_0 = dxyz_ji.dot(that_tangent);
    const auto t_ji_1 = dxyz_ji.dot(that_orth_tangent);

    return make_pair(Vector2i{t_ij_0, t_ij_1}, Vector2i{t_ji_0, t_ji_1});
}

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
) {
    using namespace std;
    using namespace Eigen;

// Compute orth tangents
    const auto this_orth_tangent = this_normal.cross(this_tangent);
    const auto that_orth_tangent = that_normal.cross(that_tangent);

    const auto this_lattice_vertex = this_vertex +
                                     this_offset[0] * this_tangent +
                                     this_offset[1] * this_orth_tangent;
    const auto that_lattice_vertex = that_vertex +
                                     that_offset[0] * that_tangent +
                                     that_offset[1] * that_orth_tangent;

// Compute q_ij and thus Q_ij and Q_ji
    const auto q = compute_qij(this_vertex, this_normal, that_vertex, that_normal);
    const auto Q_ij = compute_lattice_neighbours(this_lattice_vertex,
                                                 q,
                                                 this_tangent,
                                                 this_orth_tangent,
                                                 rho);
    const auto Q_ji = compute_lattice_neighbours(that_lattice_vertex,
                                                 q,
                                                 that_tangent,
                                                 that_orth_tangent,
                                                 rho);

// Get the closest points adjacent to q in both tangent planes
    const auto closest_points = find_closest_points(Q_ij, Q_ji);

// Compute t_ij and t_ji
    const auto dxyz_ij = (closest_points.first - this_lattice_vertex);
    t_ij = {round(dxyz_ij.dot(this_tangent)), round(dxyz_ij.dot(this_orth_tangent))};

    const auto dxyz_ji = (closest_points.second - that_lattice_vertex);
    t_ji = {round(dxyz_ji.dot(that_tangent)), round(dxyz_ji.dot(that_orth_tangent))};

    return make_pair(
            this_vertex + (t_ij[0] * this_tangent) + (t_ij[1] * this_orth_tangent),
            that_vertex + (t_ji[0] * that_tangent) + (t_ji[1] * that_orth_tangent));
}