//
// Created by Dave Durbin (Old) on 2/4/21.
//

#pragma once

#include <Surfel/SurfelGraph.h>
#include "geometry_extractor.h"

class rosy_surfel_graph_geometry_extractor
    : public geometry_extractor {
  float m_spur_length;

 public:
  rosy_surfel_graph_geometry_extractor() : geometry_extractor(), m_spur_length{1.0f} {}

  void set_spur_length(float spur_length) {
    m_spur_length = spur_length;
  }

  void extract_splats_for_frame(
      const SurfelGraphPtr &graphPtr,
      unsigned int frame,
      std::vector<float> &triangle_fan,
      std::vector<unsigned int> &triangle_fan_sizes
  );

  void extract_lines_for_frame(const SurfelGraphPtr &graphPtr,
                               unsigned int frame,
                               std::vector<float> &positions,
                               std::vector<float> &tangents,
                               std::vector<float> &normals,
                               std::vector<float> &colours
  );

  void extract_geometry(const SurfelGraphPtr &graphPtr,
                        std::vector<float> &positions,
                        std::vector<float> &tangents,
                        std::vector<float> &normals,
                        std::vector<float> &colours,
                        std::vector<float> &path,
                        std::vector<float> &triangle_fan,
                        std::vector<unsigned int> &triangle_fan_sizes,
                        float &scale_factor);

  std::vector<float> get_neighbours_for_item(
      const SurfelGraphPtr &graphPtr,
      const SurfelGraphNodePtr &node);

  void select_item_and_neighbours_at(const Eigen::Vector3f &item_vertex,
                                     std::vector<float> &selected_item_data,
                                     std::vector<float> &neighbour_data);
  void update_selection_and_neighbours(std::vector<float> &selected_item_data,
                                       std::vector<float> &neighbour_data);

 private:
  struct cmpVector {
    bool operator()(const Eigen::Vector3f &a, const Eigen::Vector3f &b) const {
      if (a[0] < b[0]) return true;
      if (a[0] > b[0]) return false;
      if (a[1] < b[1]) return true;
      if (a[1] > b[1]) return false;
      if (a[2] < b[2]) return true;
      if (a[2] > b[2]) return false;
      return false;
    }
  };
  std::map<Eigen::Vector3f, SurfelGraphNodePtr, cmpVector> m_vertex_to_node;
  SurfelGraphPtr m_graph_ptr = nullptr;

  SurfelGraphNodePtr m_selected_node = nullptr;
};
