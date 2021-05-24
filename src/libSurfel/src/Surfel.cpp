//
// Created by Dave Durbin on 19/5/20.
//

#include "Surfel.h"
#include "FrameData.h"

#include <Eigen/Core>
#include <Eigen/Geometry>
#include <utility>
#include <vector>
#include <memory>
#include <Geom/Geom.h>

std::map<std::string, std::shared_ptr<Surfel>> Surfel::m_surfel_by_id = [] {
    return std::map<std::string, std::shared_ptr<Surfel>>{};
}();

//std::shared_ptr<Surfel> Surfel::surfel_for_id(const std::string &id) {
//    auto it = m_surfel_by_id.find(id);
//    if (it != m_surfel_by_id.end()) {
//        return it->second;
//    }
//    std::cerr << "No surfel found for ID " << id << std::endl;
//    throw std::runtime_error("Bad surfel id");
//}
//
Surfel::Surfel(std::string id,
               const std::vector<FrameData> &frames,
               Eigen::Vector3f tangent,
               Eigen::Vector2f reference_lattice_offset
) :
        m_id{std::move(id)},
        m_tangent{std::move(tangent)},
        m_reference_lattice_offset{std::move(reference_lattice_offset)},
        m_rosy_smoothness{45.f * 45.f},
        m_last_rosy_correction{0.0f},
        m_posy_smoothness{0.0f} {

    for (auto &fd : frames) {
        m_frame_data.push_back(fd);
        this->m_frames.push_back(fd.pixel_in_frame.frame);
    }
}


// TODO: Consider early return is frame < fd.pif.frame when framedata is sorted
// TODO: Consider constructing vector<int> and using binary_search
bool
Surfel::is_in_frame(unsigned int frame) const {
    using namespace std;
    return any_of(begin(m_frame_data), end(m_frame_data),
                  [frame](const FrameData &fd) { return (fd.pixel_in_frame.frame == frame); });
}

const FrameData &
Surfel::frame_data_for_frame(unsigned int frame) const {
    for (const auto &fd : m_frame_data) {
        if (fd.pixel_in_frame.frame == frame) {
            return fd;
        }
    }
    throw std::runtime_error("Surfel " + m_id + " not in frame " + std::to_string(frame));
}

void
Surfel::get_vertex_tangent_normal_for_frame(
        unsigned int frame_idx,
        Eigen::Vector3f &vertex,
        Eigen::Vector3f &tangent,
        Eigen::Vector3f &normal) const {
    const auto &fd = frame_data_for_frame(frame_idx);

    vertex = fd.position;
    normal = fd.normal;
    tangent = fd.transform * m_tangent;
    // Force reproject tangent to surface
    tangent -= tangent.dot(normal) * normal;
    tangent.normalize();
}

void Surfel::get_all_data_for_surfel_in_frame(
        unsigned int frame_idx,
        Eigen::Vector3f &vertex,
        Eigen::Vector3f &tangent,
        Eigen::Vector3f &orth_tangent,
        Eigen::Vector3f &normal,
        Eigen::Vector3f &closest_mesh_vertex) const {
    get_vertex_tangent_normal_for_frame(frame_idx, vertex, tangent, normal);
    orth_tangent = normal.cross(tangent);
    closest_mesh_vertex = vertex +
                          (m_reference_lattice_offset[0] * tangent) +
                          (m_reference_lattice_offset[1] * orth_tangent);
}

void
Surfel::transform_surfel_via_frame(const std::shared_ptr<Surfel> &that_surfel_ptr,
                                   unsigned int frame_index,
                                   Eigen::Vector3f &transformed_other_norm,
                                   Eigen::Vector3f &transformed_other_tan) const {

    const auto frame_to_surfel = frame_data_for_frame(frame_index).transform.transpose();
    const auto &other_surfel_to_frame = that_surfel_ptr->frame_data_for_frame(frame_index).transform;
    auto other_surfel_to_this_surfel = frame_to_surfel * other_surfel_to_frame;

    const auto &neighbour_normal_in_frame = that_surfel_ptr->frame_data_for_frame(frame_index).normal;

    transformed_other_norm = frame_to_surfel * neighbour_normal_in_frame;
    transformed_other_tan = other_surfel_to_this_surfel * that_surfel_ptr->tangent();
}