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

void
rosy_surfel_graph_geometry_extractor::extract_lines_for_frame(const SurfelGraphPtr &graphPtr,
                                                              unsigned int frame,
                                                              std::vector<float> &positions,
                                                              std::vector<float> &tangents,
                                                              std::vector<float> &normals,
                                                              std::vector<float> &colours
) {
  m_vertex_to_node.clear();

  std::vector<float> errors;
  for (auto &node: graphPtr->nodes()) {
    const auto &surfel = node->data();
    if (!surfel->is_in_frame(frame)) {
      continue;
    }

    Eigen::Vector3f position, normal, tangent;
    surfel->get_vertex_tangent_normal_for_frame(frame, position, tangent, normal);

    m_vertex_to_node.insert({position, node});

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

std::vector<size_t>
sort_neighbours_around_centroid(
    const Eigen::Vector3f &centroid,
    const Eigen::Vector3f &normal,
    const Eigen::Vector3f &t1,
    const Eigen::Vector3f &t2,
    const std::vector<Eigen::Vector3f> &points
) {
  using namespace std;

  // Make index vector
  vector<size_t> idx(points.size());
  iota(idx.begin(), idx.end(), 0);

  // Generate atan2 for each
  std::vector<double> keys;
  for (const auto &p: points) {
    const auto r = p - centroid;
    const auto t = normal.dot(r.cross(t1));
    const auto u = normal.dot(r.cross(t2));
    keys.emplace_back(std::atan2(u, t));
  }

  stable_sort(begin(idx), end(idx),
              [&keys](const size_t v1_index, const size_t &v2_index) {
                return keys.at(v1_index) > keys.at(v2_index);
              });
  return idx;
}

size_t
find_start(
    const Eigen::Vector3f &vertex,
    const Eigen::Vector3f &normal,
    const std::vector<size_t> &ordered_indices,
    const std::vector<Eigen::Vector3f> &vertices
) {
  using namespace std;
  // Iterate through the triangles until we find one that is invalid
  // or else we get back to the start
  auto tentative_start_index = 0;
  while (tentative_start_index < ordered_indices.size()) {
    auto curr_vert_index = ordered_indices.at(tentative_start_index);
    auto next_vert_index = ordered_indices.at((tentative_start_index + 1) % ordered_indices.size());
    auto curr_vert = vertices.at(curr_vert_index);
    auto next_vert = vertices.at(next_vert_index);
    auto nn = (curr_vert - vertex).cross(next_vert - curr_vert);
    if (nn.dot(normal) >= 0) {
      // Found invalid triangle.
      break;
    }
    tentative_start_index++;
  }
  return (tentative_start_index == ordered_indices.size())
         ? 0
         : (tentative_start_index + 1) % ordered_indices.size();
}

void rosy_surfel_graph_geometry_extractor::extract_splats_for_frame(
    const SurfelGraphPtr &graphPtr,
    unsigned int frame_idx,
    std::vector<float> &triangle_fan,
    std::vector<unsigned int> &triangle_fan_sizes
) {
  using namespace std;
  using namespace Eigen;

  for (const auto &node: graphPtr->nodes()) {
    // Ignore if not in frame
    if (!node->data()->is_in_frame(frame_idx)) {
      continue;
    }

    auto neighbours = get_node_neighbours_in_frame(graphPtr, node, frame_idx);
    Vector3f vertex, tangent, normal;
    node->data()->get_vertex_tangent_normal_for_frame(frame_idx, vertex, tangent, normal);
    Vector3f nbr_vertex, nbr_tangent, nbr_normal;
    // Ignore single neighbours (for now)
    if (neighbours.size() == 1) {
      continue;
    }

    vector<Vector3f> neighbour_coords;
    for (const auto &nbr: neighbours) {
      nbr->data()->get_vertex_tangent_normal_for_frame(frame_idx, nbr_vertex, nbr_tangent, nbr_normal);
      neighbour_coords.push_back(nbr_vertex);
    }

    // Compute centroid
    Vector3f centroid;
    for (const auto &coord: neighbour_coords) {
      centroid += coord;
    }
    float divisor = 1.0f / (float) neighbour_coords.size();
    centroid *= divisor;

    // Sort them CCW around normal, returning indices
    const auto ordered_indices = sort_neighbours_around_centroid(
        vertex,
        normal,
        tangent,
        normal.cross(tangent),
        neighbour_coords
    );

    // Add all the tringgles
    int fan_size = 0;
    for (auto idx = 0; idx < ordered_indices.size(); ++idx) {
      auto curr_vert_index = ordered_indices.at(idx);
      auto next_vert_index = ordered_indices.at((idx + 1) % ordered_indices.size());
      auto curr_vert = neighbour_coords.at(curr_vert_index);
      auto next_vert = neighbour_coords.at(next_vert_index);
      auto nn = (curr_vert - vertex).cross(next_vert - curr_vert);
      if (nn.dot(normal) >= 0) {
        // Found invalid triangle, ignore it.
        continue;
      }

      triangle_fan.emplace_back(vertex[0]);
      triangle_fan.emplace_back(vertex[1]);
      triangle_fan.emplace_back(vertex[2]);

      triangle_fan.emplace_back(curr_vert[0]);
      triangle_fan.emplace_back(curr_vert[1]);
      triangle_fan.emplace_back(curr_vert[2]);

      triangle_fan.emplace_back(next_vert[0]);
      triangle_fan.emplace_back(next_vert[1]);
      triangle_fan.emplace_back(next_vert[2]);

      fan_size++;
    }
    triangle_fan_sizes.push_back(fan_size);
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
    std::vector<float> &triangle_fan,
    std::vector<unsigned int> &triangle_fan_sizes,
    float &normal_scale
) {
  positions.clear();
  tangents.clear();
  normals.clear();
  path.clear();
  colours.clear();
  triangle_fan.clear();
  triangle_fan_sizes.clear();
  m_vertex_to_node.clear();

  m_graph_ptr = graphPtr;

  extract_lines_for_frame(graphPtr, get_frame(), positions, tangents, normals, colours);
  extract_splats_for_frame(graphPtr, get_frame(), triangle_fan, triangle_fan_sizes);
  std::string vertex = "s_500";
  extract_lines_for_path(graphPtr, get_frame(), vertex, path);
  normal_scale = m_spur_length;

}

void
rosy_surfel_graph_geometry_extractor::select_item_and_neighbours_at(
    const Eigen::Vector3f &item_vertex,
    std::vector<float> &selected_item_data,
    std::vector<float> &neighbour_data
) {
  using namespace std;
  using namespace Eigen;


  auto it = m_vertex_to_node.find(item_vertex);
  if (it == m_vertex_to_node.end()) {
    m_selected_node = nullptr;
    selected_item_data.clear();
    neighbour_data.clear();
    return;
  }

  m_selected_node = it->second;
  update_selection_and_neighbours(selected_item_data, neighbour_data);
//    const auto &selected_surfel = m_selected_node->data();
//    ui->lblSurfelId->setText(QString::fromStdString(selected_surfel->id()));
//    ui->lblSurfelTanX->setText(QString::number(selected_surfel->tangent()[0]));
//    ui->lblSurfelTanY->setText(QString::number(selected_surfel->tangent()[1]));
//    ui->lblSurfelTanZ->setText(QString::number(selected_surfel->tangent()[2]));
//    ui->lblSurfelCorrection->setText(QString::number(selected_surfel->rosy_correction()));
//    ui->lblSurfelSmoothness->setText(QString::number(selected_surfel->rosy_smoothness()));
//
//    auto nbr_data = m_rosy_geometry_extractor->get_neighbours_for_item(m_graph_ptr, m_selected_node);
//    ui->rosyGLWidget->setNeighbourData(nbr_data);
}

void rosy_surfel_graph_geometry_extractor::update_selection_and_neighbours(
    std::vector<float> &selected_item_data,
    std::vector<float> &neighbour_data
) {
  using namespace Eigen;
  using namespace std;

  selected_item_data.clear();
  neighbour_data.clear();

  if (m_selected_node == nullptr) {
    return;
  }

  if (!m_selected_node->data()->is_in_frame(get_frame())) {
    return;
  }

  Vector3f vertex, tangent, normal;
  m_selected_node->data()->get_vertex_tangent_normal_for_frame(get_frame(), vertex, tangent, normal);

  // Construct the rectangle enclosing the node
  selected_item_data.push_back(vertex[0]);
  selected_item_data.push_back(vertex[1]);
  selected_item_data.push_back(vertex[2]);

  // Construct the lines to neighbours
  auto nbrs = m_graph_ptr->neighbours(m_selected_node);
  for (const auto &nbr: nbrs) {
    if (!nbr->data()->is_in_frame(get_frame())) {
      continue;
    }

    Vector3f nbr_vertex, nbr_tangent, nbr_normal;
    nbr->data()->get_vertex_tangent_normal_for_frame(get_frame(), nbr_vertex, nbr_tangent, nbr_normal);

    neighbour_data.push_back(vertex[0]);
    neighbour_data.push_back(vertex[1]);
    neighbour_data.push_back(vertex[2]);
    neighbour_data.push_back(nbr_vertex[0]);
    neighbour_data.push_back(nbr_vertex[1]);
    neighbour_data.push_back(nbr_vertex[2]);
  }
}
