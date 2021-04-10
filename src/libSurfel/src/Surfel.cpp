//
// Created by Dave Durbin on 19/5/20.
//

#include "Surfel.h"
#include "FrameData.h"

#include <Eigen/Core>
#include <utility>
#include <vector>
#include <memory>
#include <Geom/Geom.h>

std::map<std::string, std::shared_ptr<Surfel>> Surfel::surfel_by_id = [] {
    return std::map<std::string, std::shared_ptr<Surfel>>{};
}();

std::shared_ptr<Surfel> Surfel::surfel_for_id(const std::string &id) {
    auto it = surfel_by_id.find(id);
    if (it != surfel_by_id.end()) {
        return it->second;
    }
    std::cerr << "No surfel found for ID " << id << std::endl;
    throw std::runtime_error("Bad surfel id");
}

Surfel::Surfel(std::string id,
               const std::vector<FrameData> &frames,
               Eigen::Vector3f tangent,
               Eigen::Vector2f closest_mesh_vertex_offset
) :
        id{std::move(id)},
        tangent{std::move(tangent)},
        closest_mesh_vertex_offset{std::move(closest_mesh_vertex_offset)},
        last_correction{0.0f},
        error{0.0},
        posy_smoothness{0.0f} {

    for (auto &fd : frames) {
        frame_data.push_back(fd);
    }
}

// TODO: Consider early return is frame < fd.pif.frame when framedata is sorted
// TODO: Consider constructing vector<int> and using binary_search
bool
Surfel::is_in_frame(unsigned int frame) const {
    for (const auto &fd : frame_data) {
        if (fd.pixel_in_frame.frame == frame) {
            return true;
        }
    }
    return false;
}

const FrameData&
Surfel::frame_data_for_frame(unsigned int frame) const {
    for (const auto &fd : frame_data) {
        if (fd.pixel_in_frame.frame == frame) {
            return fd;
        }
    }
    throw std::runtime_error("Surfel not in frame");
}

void
Surfel::get_position_tangent_normal_for_frame(unsigned int frame, Eigen::Vector3f& position, Eigen::Vector3f& tangent, Eigen::Vector3f& normal ) const {
    const auto & fd = frame_data_for_frame(frame);

    position = fd.position;
    normal = fd.normal;
    tangent = fd.transform * this->tangent;
    // Force reproject tangent to surface
    project_vector_to_plane(tangent, normal, true);
}
