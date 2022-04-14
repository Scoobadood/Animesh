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

  for (auto &fd: frames) {
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
  for (const auto &fd: m_frame_data) {
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
  tangent.normalize();
  tangent -= tangent.dot(normal) * normal;
}

Eigen::Vector3f
Surfel::reference_lattice_vertex_in_frame(unsigned int frame_idx, float rho) const {
  const auto &fd = frame_data_for_frame(frame_idx);

  Eigen::Vector3f tangent = fd.transform * m_tangent;
  tangent.normalize();
  tangent -= tangent.dot(fd.normal) * fd.normal;
  auto orth_tangent = fd.normal.cross(tangent);
  return fd.position +
      (m_reference_lattice_offset[0] * tangent * rho) +
      (m_reference_lattice_offset[1] * orth_tangent * rho);
}


void Surfel::get_all_data_for_surfel_in_frame(
    unsigned int frame_idx,
    Eigen::Vector3f &vertex,
    Eigen::Vector3f &tangent,
    Eigen::Vector3f &orth_tangent,
    Eigen::Vector3f &normal,
    Eigen::Vector3f &closest_mesh_vertex) const {
  get_all_data_for_surfel_in_frame(frame_idx, vertex, tangent, orth_tangent, normal, closest_mesh_vertex, 0);
}

void Surfel::get_all_data_for_surfel_in_frame(
    unsigned int frame_idx,
    Eigen::Vector3f &vertex,
    Eigen::Vector3f &tangent,
    Eigen::Vector3f &orth_tangent,
    Eigen::Vector3f &normal,
    Eigen::Vector3f &closest_mesh_vertex,
    unsigned short k) const {
  get_vertex_tangent_normal_for_frame(frame_idx, vertex, tangent, normal);
  orth_tangent = normal.cross(tangent);
  // Closes mesh vertex is always computed without rotating the local frame.
  closest_mesh_vertex = vertex +
      (m_reference_lattice_offset[0] * tangent) +
      (m_reference_lattice_offset[1] * orth_tangent);

  if (k == 0) {
    return;
  }
  tangent = vector_by_rotating_around_n(tangent, normal, k);
  orth_tangent = normal.cross(tangent);
}

const Eigen::Matrix3f &Surfel::transform_for_frame(unsigned int frameIdx) const {
  for (const auto &fd: m_frame_data) {
    if (fd.pixel_in_frame.frame == frameIdx) {
      return fd.transform;
    }
  }
  throw std::runtime_error("Surfel " + m_id + " not in frame " + std::to_string(frameIdx));
}

/*
 * Transform a given surfels normal and tangent into this surfels frame of reference
 * via a specific common frame.
 */
void Surfel::transform_surfel_via_frame(
    const std::shared_ptr<Surfel> &that_surfel_ptr,
    unsigned int frame_index,
    Eigen::Vector3f &transformed_other_norm,
    Eigen::Vector3f &transformed_other_tan) const {

  // Get transform (rotation) from given frame to surfel space for this surfel
  const auto frame_to_this_surfel = frame_data_for_frame(frame_index).transform.transpose();
  const auto &that_surfel_to_frame = that_surfel_ptr->frame_data_for_frame(frame_index).transform;
  auto that_surfel_to_this_surfel = frame_to_this_surfel * that_surfel_to_frame;

  transformed_other_norm = that_surfel_to_this_surfel * Eigen::Vector3f::UnitY();
  transformed_other_tan = that_surfel_to_this_surfel * that_surfel_ptr->tangent();
}