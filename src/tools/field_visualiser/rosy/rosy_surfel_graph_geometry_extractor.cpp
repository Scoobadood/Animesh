//
// Created by Dave Durbin (Old) on 2/4/21.
//

#include "rosy_surfel_graph_geometry_extractor.h"
#include <Geom/Geom.h>

void extract_lines_for_path(const SurfelGraphPtr &graphPtr,
                            const unsigned int frame,
                            const std::string &surfel_id,
                            std::vector<float> &path) {
  bool found = false;
  std::shared_ptr<Surfel> surfel;
  for (const auto &node: graphPtr->nodes()) {
    if (node->data()->id() == surfel_id) {
      surfel = node->data();
      found = true;
      break;
    }
  }
  if (!found) {
    return;
  }

  for (auto frame_idx: surfel->frames()) {
    Eigen::Vector3f position, normal, tangent;
    surfel->get_vertex_tangent_normal_for_frame(frame_idx, position, tangent, normal);

    const auto num_frames = surfel->frames().size();
    const float fade_per_frame = 0.75f / num_frames;
    float frames_diff;
    if (frame_idx < frame) {
      frames_diff = frame - frame_idx;
    } else {
      frames_diff = frame_idx - frame;
    }
    const float frame_fade = 1.0f - (fade_per_frame * frames_diff);
    path.push_back(frame_fade); //red
    path.push_back(frame_fade); //green
    path.push_back(frame_fade); //blue
    path.push_back(1.0f); //alpha
    path.push_back(position.x());
    path.push_back(position.y());
    path.push_back(position.z());

    path.push_back(tangent.x());
    path.push_back(tangent.y());
    path.push_back(tangent.z());

    path.push_back(normal.x());
    path.push_back(normal.y());
    path.push_back(normal.z());
  }
}

void extract_lines_for_frame(const SurfelGraphPtr &graphPtr,
                             unsigned int frame,
                             std::vector<float> &positions,
                             std::vector<float> &tangents,
                             std::vector<float> &normals,
                             std::vector<float> &colours
) {
  std::vector<float> errors;
  for (const auto &node: graphPtr->nodes()) {
    const auto &surfel = node->data();
    if (!surfel->is_in_frame(frame)) {
      continue;
    }

    Eigen::Vector3f position, normal, tangent;
    surfel->get_vertex_tangent_normal_for_frame(frame, position, tangent, normal);

    positions.push_back(position.x());
    positions.push_back(position.y());
    positions.push_back(position.z());

    tangents.push_back(tangent.x());
    tangents.push_back(tangent.y());
    tangents.push_back(tangent.z());

    normals.push_back(normal.x());
    normals.push_back(normal.y());
    normals.push_back(normal.z());

    errors.push_back(surfel->rosy_smoothness());
  }
  float max_error = 0.0f;
  for (float error: errors) {
    if (error > max_error) {
      max_error = error;
    }
  }
  // Now scale errors
  auto error_scale_factor = (max_error != 0.0f)
                            ? (1.0f / max_error)
                            : 0.0f;
  for (float error: errors) {
    auto err = error * error_scale_factor;
    colours.push_back(err); // red
    colours.push_back(1.0f - err); // grn
    colours.push_back(0.0f); // blu
    colours.push_back(1.0f); // alp
  }
}

float compute_normal_scale(
    const SurfelGraphPtr &graphPtr,
    float model_scale) {
  // Pick a surfel and compute mean neighbour distance for this frame
  const auto node = graphPtr->nodes().front();
  const auto surfel = node->data();
  const auto neighbours = graphPtr->neighbours(node);
  int count = 0;
  float distance = 0.0f;
  for (const auto &fd: surfel->frame_data()) {
    unsigned int frame = fd.pixel_in_frame.frame;
    const auto &this_surfel_position = fd.position;
    for (const auto &neighbour_node: neighbours) {
      const auto &otherSurfel = neighbour_node->data();
      if (otherSurfel->is_in_frame(frame)) {
        Eigen::Vector3f other_surfel_position, tangent, normal;
        otherSurfel->get_vertex_tangent_normal_for_frame(frame, other_surfel_position, tangent, normal);
        distance += (distance_from_point_to_point(other_surfel_position, this_surfel_position));
        count++;
      }
    }
  }

  const auto mean_neighbour_distance = (count > 0)
                                       ? distance / (float) count
                                       : 1.0f;

  // Proposed scale should be 2/5 of mean neighbour distance so that
  // in general, adjacent tangents don't touch. But we must also scale by the model_scale
  return mean_neighbour_distance * model_scale * 0.4f;
}

/**
 * @brief Generate the vertex data needed to render the current frame of this graph.
 * All float vectors are populated in x,y,z coordinate triplets.
 * @param graph The Surfel Graph
 * @param positions a vector into which the Surfel positions will be pushed.
 * @param tangents A Vector of XYZ coordinates for the unit indicative tangent.
 * @param normals A Vector of XYZ coordinates for the unit normal.
 * @param scaleFactor A proposed scaling for the normals and tangents.
 */
void rosy_surfel_graph_geometry_extractor::extract_geometry(
    const SurfelGraphPtr &graphPtr,
    std::vector<float> &positions,
    std::vector<float> &tangents,
    std::vector<float> &normals,
    std::vector<float> &colours,
    std::vector<float> &path,
    float &normal_scale
) const {
  positions.clear();
  tangents.clear();
  normals.clear();
  path.clear();
  colours.clear();

  extract_lines_for_frame(graphPtr, get_frame(), positions, tangents, normals, colours);
  std::string vertex = "s_500";
  extract_lines_for_path(graphPtr, get_frame(), vertex, path);
  normal_scale = m_spur_length;
}
