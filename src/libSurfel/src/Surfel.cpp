//
// Created by Dave Durbin on 19/5/20.
//

#include "Surfel.h"
#include "FrameData.h"

#include <Eigen/Core>
#include <utility>
#include <vector>
#include <string>
#include <memory>

std::map<std::string, std::shared_ptr<Surfel>> Surfel::surfel_by_id = []{
    return std::map<std::string, std::shared_ptr<Surfel>>{};
}();

std::shared_ptr<Surfel> Surfel::surfel_for_id(const std::string& id) {
    auto it = surfel_by_id.find(id);
    if( it != surfel_by_id.end() ) {
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
