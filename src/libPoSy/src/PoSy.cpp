
#include <PoSy/PoSy.h>
#include <Geom/Geom.h>
#include <Eigen/Core>
#include <Eigen/Geometry>
#include <vector>
#include <tuple>
#include <spdlog/spdlog.h>


/**
 * Given a point in space and a point on the lattice, return the coordinates of the
 * four surrounding lattice points in 3-space.
 */
/**
 * Given a point in space, a normal, tangent vector and rho (lattice spacing) compute the 4 nearest vertices on the lattice
 */
std::vector<Eigen::Vector3f> compute_lattice_neighbours(
        const Eigen::Vector3f &lattice_point,
        const Eigen::Vector3f &point,
        const Eigen::Vector3f &tangent,
        const Eigen::Vector3f &orth_tangent,
        float rho) {
    using namespace std;
    using namespace Eigen;

    const auto inv_rho = 1.0f / rho;
    vector<Vector3f> neighbours;

    const auto diff = point - lattice_point;

    const auto base = lattice_point +
            (tangent * std::floorf(diff.dot(tangent) * inv_rho) * rho) +
            (orth_tangent * std::floorf(diff.dot(orth_tangent) * inv_rho) * rho);

    for (int u = 0; u <= 1; ++u) {
        for (int v = 0; v <= 1; ++v) {
            neighbours.emplace_back(base + (u * tangent * rho) + (v * orth_tangent * rho));
        }
    }

    return neighbours;
}

/**
 * Given a regular grid with orientation tangent_u,
 * position gridPosition and n, the following operation rounds a
 * position roundPosition′ to the nearest lattice point.
 * @param gridPosition
 * @param roundPosition
 * @param tangent_u
 * @param tangent_v
 * @param n
 * @param rho
 * @return
 */
Eigen::Vector3f
round_4(const Eigen::Vector3f &n,
        const Eigen::Vector3f &o,
        const Eigen::Vector3f &p,
        const Eigen::Vector3f &q,
        float rho) {
    const auto inv_rho = 1.0f / rho;

    const auto o_prime = n.cross(o);
    const auto diff = (q - p);
    return p
           + o * (std::roundf(diff.dot(o) * inv_rho) * rho)
           + o_prime * (std::roundf(diff.dot(o_prime) * inv_rho) * rho);
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
 * Compute the midpoint between two points on planes defined by normal vectors
 * n_i and n_j. The midpoint is forced to lie on the line of intersection of the planes.
 * @param v_i Point on first plane.
 * @param n_i Normal of first plane.
 * @param v_j Point on second plane.
 * @param n_j Nrmal of second plane.
 * @return
 */
Eigen::Vector3f compute_qij(
        const Eigen::Vector3f &v_i,
        const Eigen::Vector3f &n_i,
        const Eigen::Vector3f &v_j,
        const Eigen::Vector3f &n_j
) {
    const auto nivi = n_i.dot(v_i);
    const auto nivj = n_i.dot(v_j);
    const auto njvi = n_j.dot(v_i);
    const auto njvj = n_j.dot(v_j);
    const auto ninj = n_i.dot(n_j);
    const auto denom = 1.0f / (1.0f - ninj * ninj + 1e-4f);

    const auto lambda_i = 2.0f * (nivj - nivi - ninj * (njvi - njvj)) * denom;
    const auto lambda_j = 2.0f * (njvi - njvj - ninj * (nivj - nivi)) * denom;

    const auto q_ij = ((v_i + v_j) * 0.5f) - (((lambda_i * n_i) + (lambda_j * n_j)) * 0.25f);
    return q_ij;
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
    float best_dist = INFINITY;
    Eigen::Vector3f best_pa, best_pb;
    for (const auto &p_a : points_a) {
        for (const auto &p_b : points_b) {
            const auto dist = (p_b - p_a).squaredNorm();
            if (dist < best_dist) {
                best_pa = p_a;
                best_pb = p_b;
                best_dist = dist;
            }
        }
    }
    return std::make_pair(best_pa, best_pb);
}