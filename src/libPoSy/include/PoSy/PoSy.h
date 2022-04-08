#pragma once

#include <Eigen/Core>

Eigen::Vector3f position_round(const Eigen::Vector3f &anchor,
                               const Eigen::Vector3f &tangent,
                               const Eigen::Vector3f &orth_tangent,
                               const Eigen::Vector3f &point,
                               float scale);

Eigen::Vector3f
compute_qij(
    const Eigen::Vector3f &vertex,
    const Eigen::Vector3f &normal,
    const Eigen::Vector3f &other_vertex,
    const Eigen::Vector3f &other_normal);

std::pair<Eigen::Vector3f, Eigen::Vector3f> compute_closest_lattice_points(
    const Eigen::Vector3f &vertex,
    const Eigen::Vector3f &normal,
    const Eigen::Vector3f &tangent,
    const Eigen::Vector3f &orth_tangent,
    const Eigen::Vector3f &lattice_vertex,
    const Eigen::Vector3f &other_vertex,
    const Eigen::Vector3f &other_normal,
    const Eigen::Vector3f &other_tangent,
    const Eigen::Vector3f &othert_orth_tangent,
    const Eigen::Vector3f &other_lattice_vertex,
    float scale);

std::pair<Eigen::Vector2i, Eigen::Vector2i> compute_tij_tji(
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
    float scale,
    float *error = nullptr);
