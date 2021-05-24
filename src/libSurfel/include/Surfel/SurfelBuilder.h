//
// Created by Dave Durbin (Old) on 21/5/21.
//

#pragma once

#include <string>
#include <random>
#include <Eigen/Eigen>
#include "Surfel.h"

class SurfelBuilder {
public:
    explicit SurfelBuilder(std::default_random_engine& random_engine);

    SurfelBuilder *reset();

    SurfelBuilder *with_id(const std::string &id);

    SurfelBuilder *with_tangent(const Eigen::Vector3f &tangent);

    SurfelBuilder *with_tangent(float x, float y, float z);

    SurfelBuilder *with_reference_lattice_offset(const Eigen::Vector2f &reference_lattice_offset);

    SurfelBuilder *with_reference_lattice_offset(float u, float v);

    SurfelBuilder *with_frame(const FrameData &frame_data);

    SurfelBuilder *with_frame(const PixelInFrame &pif,
                              float depth,
                              const Eigen::Matrix3f &tran,
                              const Eigen::Vector3f &norm,
                              const Eigen::Vector3f &pos);

    SurfelBuilder *with_frame(const PixelInFrame &pif,
                              float depth,
                              const Eigen::Vector3f &norm,
                              const Eigen::Vector3f &pos);

    Surfel build();

private:
    static const Eigen::Vector3f Y_AXIS;

    // Random distributions
    std::uniform_real_distribution<float> m_two_pi;
    std::uniform_real_distribution<float> m_unit;

    std::string m_id;
    std::vector<FrameData> m_frames;
    Eigen::Vector3f m_tangent;
    Eigen::Vector2f m_reference_lattice_offset;
    std::default_random_engine& m_random_engine;

    bool m_id_set;
    bool m_tangent_set;
    bool m_reference_lattice_offset_set;

    void generate_random_tangent();

    void generate_random_reference_lattice_offset();

    void generate_default_frame();
    void generate_random_id();
};