//
// Created by Dave Durbin (Old) on 21/5/21.
//

#include "SurfelBuilder.h"

#include <string>
#include <utility>
#include <Geom/Geom.h>

const Eigen::Vector3f SurfelBuilder::Y_AXIS = {0.0f, 1.0f, 0.0f};

SurfelBuilder::SurfelBuilder(std::mt19937 & random_engine)
        : m_two_pi{-M_PI, M_PI} //
        , m_unit(-0.5f, 0.5f) //
        , m_random_engine(random_engine) //
        , m_id_set{false} //
        , m_tangent_set{false} //
        , m_reference_lattice_offset_set{false} //
{
}

SurfelBuilder *SurfelBuilder::reset() {
    m_id_set = false;
    m_tangent_set = false;
    m_reference_lattice_offset_set = false;
    m_frames.clear();
    return this;
}

SurfelBuilder *SurfelBuilder::with_id(const std::string &id) {
    m_id = id;
    m_id_set = true;
    return this;
}

SurfelBuilder *SurfelBuilder::with_tangent(const Eigen::Vector3f &tangent) {
    assert(tangent.isOrthogonal(Y_AXIS));
    assert(tangent.isUnitary());
    m_tangent = tangent;
    m_tangent_set = true;
    return this;
}

SurfelBuilder *SurfelBuilder::with_tangent(float x, float y, float z) {
    return with_tangent({x, y, z});
}

SurfelBuilder *SurfelBuilder::with_reference_lattice_offset(const Eigen::Vector2f &reference_lattice_offset) {
    m_reference_lattice_offset = reference_lattice_offset;
    m_reference_lattice_offset_set = true;
    return this;
}

SurfelBuilder *SurfelBuilder::with_reference_lattice_offset(float u, float v) {
    return with_reference_lattice_offset({u, v});
}


SurfelBuilder *SurfelBuilder::with_frame(const FrameData &frame_data) {
    m_frames.emplace_back(frame_data);
    return this;
}

SurfelBuilder *SurfelBuilder::with_frame(const PixelInFrame &pif,
                                         float depth,
                                         const Eigen::Matrix3f &tran,
                                         const Eigen::Vector3f &norm,
                                         const Eigen::Vector3f &pos) {
    m_frames.emplace_back(pif, depth, tran, norm, pos);
    return this;
}

SurfelBuilder *SurfelBuilder::with_frame( //
        const PixelInFrame &pif, //
        float depth, //
        const Eigen::Vector3f &norm, //
        const Eigen::Vector3f &pos) {
    Eigen::Matrix3f transform;
    // Transform Y axis into normal
    auto c = Eigen::Vector3f{0, 1, 0}.dot(norm);
    if (c == 1) {
        transform = Eigen::Matrix3f::Identity();
    } else if (c == -1) {
        transform << 1, 0, 0, 0, -1, 0, 0, 0, 1;
    } else {
        auto v = Eigen::Vector3f{0, 1, 0}.cross(norm);
        auto s = v.norm();
        auto skew = skew_symmetrix_matrix_for(v);
        transform = Eigen::Matrix3f::Identity() + skew + ((skew * skew) * ((1.0 - c) / (s * s)));
    }

    return with_frame(pif, depth, transform, norm, pos);
}

void
SurfelBuilder::generate_random_id() {
    using namespace std;

    uniform_int_distribution<int> dist(0, 15);
    const char *v = "0123456789abcdef";
    const bool dash[] = {false, false, false, false, false, false, false, false};

    string res;
    for (bool i : dash) {
        if (i) res += "-";
        res += v[dist(m_random_engine)];
        res += v[dist(m_random_engine)];
    }
    m_id = res;
    m_id_set = true;
}

void SurfelBuilder::generate_default_frame() {
    Eigen::Matrix3f identity;
    identity.setIdentity();

    m_frames.emplace_back(
            PixelInFrame{0, 0, 0},
            0.0f,
            identity,
            Y_AXIS,
            Eigen::Vector3f{0, 0, 0});
}

void SurfelBuilder::generate_random_tangent() {
    auto theta = m_two_pi(m_random_engine);
    m_tangent = {std::cosf(theta), 0, std::sinf(theta)};
}

void SurfelBuilder::generate_random_reference_lattice_offset() {
    m_reference_lattice_offset = {
            m_unit(m_random_engine),
            m_unit(m_random_engine)};
}

Surfel SurfelBuilder::build() {
    if( !m_id_set) {
        generate_random_id();
    }
    if (!m_tangent_set) {
        generate_random_tangent();
    }
    if (!m_reference_lattice_offset_set) {
        generate_random_reference_lattice_offset();
    }
    if (m_frames.empty()) {
        generate_default_frame();
    }
    return Surfel{m_id,
                  m_frames,
                  m_tangent,
                  m_reference_lattice_offset};
}
