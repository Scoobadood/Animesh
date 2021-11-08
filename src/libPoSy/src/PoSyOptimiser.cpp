//
// Created by Dave Durbin on 18/5/20.
//

#include "PoSyOptimiser.h"

#include <spdlog/spdlog.h>
#include <PoSy.h>
#include <Geom/Geom.h>
#include <Eigen/Geometry>
#include <Surfel/SurfelGraph.h>

const std::string WATCH_NODE = "s_3_0";

PoSyOptimiser::PoSyOptimiser(const Properties &properties, std::mt19937 &rng)
    : NodeOptimiser{properties, rng} //
{
  m_rho = m_properties.getFloatProperty("rho");

  setup_termination_criteria(
      "posy-termination-criteria",
      "posy-term-crit-relative-smoothness",
      "posy-term-crit-absolute-smoothness",
      "posy-term-crit-max-iterations");
  m_randomise_neighour_order = m_properties.getBooleanProperty("posy-randomise-neighbour-order");

  setup_ssa();
}

void
PoSyOptimiser::trace_smoothing(const SurfelGraphPtr &surfel_graph) const {
  spdlog::trace("Round completed.");
  for (const auto &n: surfel_graph->nodes()) {
    Eigen::Vector3f vertex, tangent, normal;
    if (n->data()->is_in_frame(0)) {
      const auto &ref_lat_offset = n->data()->reference_lattice_offset();
      n->data()->get_vertex_tangent_normal_for_frame(0, vertex, tangent, normal);

      const auto ref_lat_vertex = vertex
          + (ref_lat_offset[0] * tangent)
          + (ref_lat_offset[1] * (normal.cross(tangent)));

      spdlog::trace("  p : ({:6.3f}, {:6.3f}, {:6.3f}),  d : ({:6.3f}, {:6.3f}), nl : ({:6.3f}, {:6.3f}, {:6.3f})",
                    vertex[0], vertex[1], vertex[2],
                    n->data()->posy_correction()[0],
                    n->data()->posy_correction()[1],
                    ref_lat_vertex[0],
                    ref_lat_vertex[1],
                    ref_lat_vertex[2]
      );
    }
  }
}

float
PoSyOptimiser::compute_node_smoothness_for_frame(const SurfelGraphNodePtr &node_ptr,
                                                 size_t frame_index,
                                                 unsigned int &num_neighbours) const {
  float frame_smoothness = 0.0f;

  const auto neighbours_in_frame = get_neighbours_of_node_in_frame(m_surfel_graph, node_ptr, frame_index, false);

  // For each neighbour...
  for (const auto &neighbour_node: neighbours_in_frame) {

    Eigen::Vector3f vertex, normal, tangent;
    node_ptr->data()->get_vertex_tangent_normal_for_frame(
        frame_index,
        vertex,
        tangent,
        normal
    );

    Eigen::Vector3f nbr_vertex, nbr_normal, nbr_tangent;
    neighbour_node->data()->get_vertex_tangent_normal_for_frame(
        frame_index,
        nbr_vertex,
        nbr_tangent,
        nbr_normal);

    // Get edge data
//    const auto &edge = m_surfel_graph->edge(node_ptr, neighbour_node);
    const auto k = get_k(m_surfel_graph, node_ptr, neighbour_node);
    const auto k_ij = k.first;
    const auto k_ji = k.second;

    // Orient tangents appropriately for frame based on k_ij and k_ji
    const auto &oriented_tangent = vector_by_rotating_around_n(tangent, normal, k_ij);
    const auto &nbr_oriented_tangent = vector_by_rotating_around_n(nbr_tangent, nbr_normal, k_ji);

    // Compute orth tangents
    const auto &orth_tangent = normal.cross(oriented_tangent);
    const auto &nbr_orth_tangent = nbr_normal.cross(nbr_oriented_tangent);

    // Compute lattice points for this node and neighbour
    Eigen::Vector3f nearest_lattice_point = vertex +
        node_ptr->data()->reference_lattice_offset().x() * oriented_tangent +
        node_ptr->data()->reference_lattice_offset().y() * orth_tangent;
    const auto nbr_nearest_lattice_point = nbr_vertex +
        neighbour_node->data()->reference_lattice_offset().x() *
            nbr_oriented_tangent +
        neighbour_node->data()->reference_lattice_offset().y() *
            nbr_orth_tangent;

    // Compute q_ij ... the midpoint on the intersection of the tangent planes
    const auto q = compute_qij(vertex, normal, nbr_vertex, nbr_normal);
    const auto Q_ij = compute_lattice_neighbours(nearest_lattice_point,
                                                 q,
                                                 oriented_tangent,
                                                 orth_tangent,
                                                 m_rho);
    const auto Q_ji = compute_lattice_neighbours(nbr_nearest_lattice_point,
                                                 q,
                                                 nbr_oriented_tangent,
                                                 nbr_orth_tangent,
                                                 m_rho);

    const auto closest_pair = find_closest_points(Q_ij, Q_ji);
    const auto cp_i = closest_pair.first;
    const auto cp_j = closest_pair.second;

    // Compute the smoothness over this surfel in this frame and the neighbour in this frame.
    const auto delta = (cp_j - cp_i).squaredNorm();
    if (m_properties.getBooleanProperty("diagnose_dodgy_deltas")) {
      if (delta > 0.866) {
        spdlog::warn("Unlikely looking closest distance {:4} for surfels {} and {}", delta,
                     node_ptr->data()->id(),
                     neighbour_node->data()->id());
        spdlog::warn("v_i = [{:3}, {:3}, {:3}]", vertex[0], vertex[1], vertex[2]);
        spdlog::warn("tan_i = [{:3}, {:3}, {:3}]", tangent[0], tangent[1], tangent[2]);
        spdlog::warn("otan_i = [{:3}, {:3}, {:3}]", orth_tangent[0], orth_tangent[1], orth_tangent[2]);
        spdlog::warn("n_i = [{:3f}, {:3f}, {:3f}]", normal[0], normal[1], normal[2]);
        spdlog::warn("v_j = [{:3}, {:3}, {:3}]", nbr_vertex[0], nbr_vertex[1], nbr_vertex[2]);
        spdlog::warn("tan_j = [{:3}, {:3}, {:3}]", nbr_tangent[0], nbr_tangent[1], nbr_tangent[2]);
        spdlog::warn("otan_j = [{:3}, {:3}, {:3}]", nbr_orth_tangent[0], nbr_orth_tangent[1],
                     nbr_orth_tangent[2]);
        spdlog::warn("n_j = [{:3f}, {:3f}, {:3f}]", nbr_normal[0], nbr_normal[1], nbr_normal[2]);
        spdlog::warn("q_ij = [{:3f}, {:3f}, {:3f}]", q[0], q[1], q[2]);
        spdlog::warn("Qij = [{:3f}, {:3f}, {:3f};", Q_ij[0][0], Q_ij[0][1], Q_ij[0][2]);
        spdlog::warn("       {:3f}, {:3f}, {:3f};", Q_ij[1][0], Q_ij[1][1], Q_ij[1][2]);
        spdlog::warn("       {:3f}, {:3f}, {:3f};", Q_ij[3][0], Q_ij[3][1], Q_ij[3][2]);
        spdlog::warn("       {:3f}, {:3f}, {:3f};", Q_ij[2][0], Q_ij[2][1], Q_ij[2][2]);
        spdlog::warn("       {:3f}, {:3f}, {:3f}]", Q_ij[0][0], Q_ij[0][1], Q_ij[0][2]);
        spdlog::warn("Qji = [{:3f}, {:3f}, {:3f};", Q_ji[0][0], Q_ji[0][1], Q_ji[0][2]);
        spdlog::warn("       {:3f}, {:3f}, {:3f};", Q_ji[1][0], Q_ji[1][1], Q_ji[1][2]);
        spdlog::warn("       {:3f}, {:3f}, {:3f};", Q_ji[3][0], Q_ji[3][1], Q_ji[3][2]);
        spdlog::warn("       {:3f}, {:3f}, {:3f};", Q_ji[2][0], Q_ji[2][1], Q_ji[2][2]);
        spdlog::warn("       {:3f}, {:3f}, {:3f}]", Q_ji[0][0], Q_ji[0][1], Q_ji[0][2]);
        spdlog::warn("cl_i = [{:3f}, {:3f}, {:3f}]", cp_i[0], cp_i[1], cp_i[2]);
        spdlog::warn("cl_j = [{:3f}, {:3f}, {:3f}]", cp_j[0], cp_j[1], cp_j[2]);

        // Do some checking
        spdlog::warn("n_i . (Q_i3 - Q_i1) is {:4}", normal.dot(Q_ij[3] - Q_ij[1]));
        spdlog::warn("n_j . (Q_j3 - Q_j1) is {:4}", nbr_normal.dot(Q_ji[3] - Q_ji[1]));
      }
    }
    frame_smoothness += delta;
  }
  num_neighbours = neighbours_in_frame.size();
  return frame_smoothness;
}

/**
 * Comparator that sorts two SurfelGRaphNodePtrs based on the smoothness
 * with the largest smoothness first.
 */
bool
PoSyOptimiser::compare_worst_first(const SurfelGraphNodePtr &l, const SurfelGraphNodePtr &r) const {
  return l->data()->posy_smoothness() > r->data()->posy_smoothness();
}

Eigen::Vector2i
position_floor_index(
    const Eigen::Vector3f &lattice_point,
    const Eigen::Vector3f &tangent,
    const Eigen::Vector3f &orth_tangent,
    const Eigen::Vector3f &midpoint,
    float scale
) {
  using namespace Eigen;
  auto inv_scale = 1.0f / scale;

  Vector3f delta = midpoint - lattice_point;
  return {
      (int) std::floor(tangent.dot(delta) * inv_scale),
      (int) std::floor(orth_tangent.dot(delta) * inv_scale)};
}

Eigen::Vector3f
position_floor(
    const Eigen::Vector3f &lattice_point,
    const Eigen::Vector3f &tangent,
    const Eigen::Vector3f &orth_tangent,
    const Eigen::Vector3f &midpoint,
    float scale
) {
  using namespace Eigen;
  using namespace std;
  auto inv_scale = 1.0f / scale;

  Vector3f d = midpoint - lattice_point;
  // Computes the 'bottom left' lattice point closest to midpoint
  return lattice_point +
      tangent * floorf(tangent.dot(d) * inv_scale) * scale +
      orth_tangent * floorf(orth_tangent.dot(d) * inv_scale) * scale;
}

std::pair<Eigen::Vector2i, Eigen::Vector2i>
compute_tij_pair(
    const Eigen::Vector3f &origin,
    const Eigen::Vector3f &tangent,
    const Eigen::Vector3f &orth_tangent,
    const Eigen::Vector3f &nbr_origin,
    const Eigen::Vector3f &nbr_tangent,
    const Eigen::Vector3f &nbr_orth_tangent,
    const Eigen::Vector3f &midpoint,
    float scale
) {
  using namespace Eigen;
  using namespace std;

  auto base_lattice_offset = position_floor_index(origin, tangent, orth_tangent, midpoint, scale);
  auto nbr_base_lattice_offset = position_floor_index(nbr_origin, nbr_tangent, nbr_orth_tangent, midpoint, scale);

  float best_cost = numeric_limits<float>::infinity();
  int best_i = -1, best_j = -1;
  for (int i = 0; i < 4; ++i) {
    Vector3f o0t = origin
        + (tangent * ((i & 1) + base_lattice_offset[0]) +
            orth_tangent * (((i & 2) >> 1) + base_lattice_offset[1]))
            * scale;
    for (int j = 0; j < 4; ++j) {
      Vector3f o1t = nbr_origin
          + (nbr_tangent * ((j & 1) + nbr_base_lattice_offset[0])
              + nbr_orth_tangent * (((j & 2) >> 1) + nbr_base_lattice_offset[1]))
              * scale;
      float cost = (o0t - o1t).squaredNorm();

      if (cost < best_cost) {
        best_i = i;
        best_j = j;
        best_cost = cost;
      }
    }
  }
  return std::make_pair(
      Vector2i((best_i & 1) + base_lattice_offset[0], ((best_i & 2) >> 1) + base_lattice_offset[1]),
      Vector2i((best_j & 1) + nbr_base_lattice_offset[0], ((best_j & 2) >> 1) + nbr_base_lattice_offset[1]));
}

std::pair<Eigen::Vector3f, Eigen::Vector3f>
compute_closest_points(
    const Eigen::Vector3f &lattice_point,
    const Eigen::Vector3f &tangent,
    const Eigen::Vector3f &orth_tangent,
    const Eigen::Vector3f &nbr_lattice_point,
    const Eigen::Vector3f &nbr_tangent,
    const Eigen::Vector3f &nbr_orth_tangent,
    const Eigen::Vector3f &midpoint,
    float scale,
    std::vector<Eigen::Vector3f> &i_vecs,
    std::vector<Eigen::Vector3f> &j_vecs
) {
  using namespace Eigen;
  using namespace std;

  // Origin, reptan, normal, point
  Vector3f this_base_lattice_point = position_floor(lattice_point, tangent, orth_tangent, midpoint, scale);
  Vector3f that_base_lattice_point =
      position_floor(nbr_lattice_point, nbr_tangent, nbr_orth_tangent, midpoint, scale);

  float best_cost = numeric_limits<float>::infinity();
  int best_i = -1, best_j = -1;

  for (int i = 0; i < 4; ++i) {
    Vector3f
        test_this_lattice_point = this_base_lattice_point + (tangent * (i & 1) + orth_tangent * ((i & 2) >> 1)) * scale;
    i_vecs.push_back(test_this_lattice_point);
    Vector3f test_that_lattice_point =
        that_base_lattice_point + (nbr_tangent * (i & 1) + nbr_orth_tangent * ((i & 2) >> 1)) * scale;
    j_vecs.push_back(test_that_lattice_point);
  }
  for (int i = 0; i < 4; ++i) {
    // Derive ,test_this_lattice_point (test)  sequential, the other bounds of 1_ij in first plane
    Vector3f
        test_this_lattice_point = this_base_lattice_point + (tangent * (i & 1) + orth_tangent * ((i & 2) >> 1)) * scale;

    for (int j = 0; j < 4; ++j) {
      Vector3f test_that_lattice_point =
          that_base_lattice_point + (nbr_tangent * (j & 1) + nbr_orth_tangent * ((j & 2) >> 1)) * scale;
      float cost = (test_this_lattice_point - test_that_lattice_point).squaredNorm();

      if (cost < best_cost) {
        best_i = i;
        best_j = j;
        best_cost = cost;
      }
    }
  }

  // best_i and best_j are the closest vertices surrounding q_ij
  // we return the pair of closest points on the lattice according to both origins
  return std::make_pair(
      this_base_lattice_point + (tangent * (best_i & 1) + orth_tangent * ((best_i & 2) >> 1)) * scale,
      that_base_lattice_point + (nbr_tangent * (best_j & 1) + nbr_orth_tangent * ((best_j & 2) >> 1)) * scale);
}

std::pair<Eigen::Vector3f, Eigen::Vector3f>
compute_closest_points(
    const Eigen::Vector3f &lattice_point,
    const Eigen::Vector3f &tangent,
    const Eigen::Vector3f &orth_tangent,
    const Eigen::Vector3f &nbr_lattice_point,
    const Eigen::Vector3f &nbr_tangent,
    const Eigen::Vector3f &nbr_orth_tangent,
    const Eigen::Vector3f &midpoint,
    float scale
) {
  std::vector<Eigen::Vector3f> i_vecs;
  std::vector<Eigen::Vector3f> j_vecs;
  return compute_closest_points(
      lattice_point, tangent, orth_tangent,
      nbr_lattice_point, nbr_tangent, nbr_orth_tangent,
      midpoint, scale, i_vecs, j_vecs);
}

/**
 * Optimise this GraphNode by considering all neighbours and allowing them all to
 * 'push' this node slightly to an agreed common position.
 * @param node
 */
void
PoSyOptimiser::optimise_node(const SurfelGraphNodePtr &node) {
  using namespace Eigen;

  const auto &curr_surfel = node->data();
  Vector2f curr_lattice_offset = curr_surfel->reference_lattice_offset();

  // For each frame in which this surfel appears
  for (const auto frame_idx: curr_surfel->frames()) {

    // Compute the current vertex 3D position of nearest lattice vertex in this frame
    Vector3f vertex, tangent, normal;
    curr_surfel->get_vertex_tangent_normal_for_frame(frame_idx, vertex, tangent, normal);
    Vector3f orth_tangent = normal.cross(tangent);
    Vector3f curr_lattice_vertex = vertex +
        curr_lattice_offset[0] * tangent +
        curr_lattice_offset[1] * orth_tangent;

    // Consider every neighbour in this frame
    const auto neighbours_in_frame =
        get_neighbours_of_node_in_frame(m_surfel_graph, node, frame_idx, m_randomise_neighour_order);

    float sum_w = 1.0f;
    for (const auto &nbr_node: neighbours_in_frame) {
      const auto &nbr_surfel = nbr_node->data();
      Vector2f nbr_lattice_offset = nbr_surfel->reference_lattice_offset();
      // Collect neighbour data
      Vector3f nbr_vertex, nbr_tangent, nbr_normal;
      nbr_surfel->get_vertex_tangent_normal_for_frame(frame_idx, nbr_vertex, nbr_tangent, nbr_normal);
      Vector3f nbr_orth_tangent = nbr_normal.cross(nbr_tangent);
      Vector3f nbr_lattice_vertex = nbr_vertex +
          nbr_lattice_offset[0] * nbr_tangent +
          nbr_lattice_offset[1] * nbr_orth_tangent;

      // Get edge data for the edge from this node to this neighbour
      const auto k = get_k(m_surfel_graph, node, nbr_node);
      const auto k_ij = k.first;
      const auto k_ji = k.second;
      const auto w_ij = 1.0f;

      // Compute edge adjusted tangents for this node and neighbour
      Vector3f edge_adjusted_tangent = (k_ij == 0)
                                       ? tangent
                                       : vector_by_rotating_around_n(tangent, normal, k_ij);
      Vector3f edge_adjusted_orth_tangent = normal.cross(edge_adjusted_tangent);

      Vector3f nbr_edge_adjusted_tangent = (k_ji == 0)
                                           ? nbr_tangent
                                           : vector_by_rotating_around_n(nbr_tangent, nbr_normal, k_ji);
      Vector3f nbr_edge_adjusted_orth_tangent = nbr_normal.cross(nbr_edge_adjusted_tangent);

      // Compute the midpoint
      const auto midpoint = compute_qij(vertex, normal, nbr_vertex, nbr_normal);
      const auto t_ij_pair = compute_tij_pair(
          curr_lattice_vertex, edge_adjusted_tangent, edge_adjusted_orth_tangent,
          nbr_lattice_vertex, nbr_edge_adjusted_tangent, nbr_edge_adjusted_orth_tangent,
          midpoint, m_rho);

      const auto closest_points = compute_closest_points(
          curr_lattice_vertex, edge_adjusted_tangent, edge_adjusted_orth_tangent,
          nbr_lattice_vertex, nbr_edge_adjusted_tangent, nbr_edge_adjusted_orth_tangent,
          midpoint, m_rho
      );

      set_t(m_surfel_graph, node, t_ij_pair.first, nbr_node, t_ij_pair.second);

      // Compute the weighted mean closest point
      curr_lattice_vertex = (sum_w * closest_points.first) + (w_ij * closest_points.second);
      sum_w += w_ij;
      curr_lattice_vertex = curr_lattice_vertex * (1.0f / sum_w);
      // Push back to tangent plane.
      curr_lattice_vertex -= normal.dot(curr_lattice_vertex - vertex) * normal;

      // Make sure this is actually the closest lattice vertex by doing the rounding thing
      curr_lattice_vertex = round_4(normal, tangent, curr_lattice_vertex, vertex, m_rho);
    } // End of neighbours

    // Pick the closest estimated lattice vertex based on CLV
    curr_lattice_vertex = round_4(normal, tangent, curr_lattice_vertex, vertex, m_rho);
    // Convert back to an offset at k=0
    const auto diff = curr_lattice_vertex - vertex;
    curr_lattice_offset = {diff.dot(tangent), diff.dot(orth_tangent)};
    if ((std::fabsf(curr_lattice_offset[0]) - 0.5 > 1e-5) ||
        (std::fabsf(curr_lattice_offset[1]) - 0.5 > 1e-5)) {
      spdlog::error("curr_lattice_offset is too far away {},{}",
                    curr_lattice_offset[0],
                    curr_lattice_offset[1]);
    }
    curr_surfel->set_reference_lattice_offset(curr_lattice_offset);
  } // End of frames
}

void
PoSyOptimiser::loaded_graph() {
  if (!m_properties.hasProperty("posy-offset-intialisation")) {
    return;
  }

  std::vector<float> initialisation_vector = m_properties.getListOfFloatProperty("posy-offset-intialisation");
  for (auto &node: m_surfel_graph->nodes()) {
    node->data()->set_reference_lattice_offset({initialisation_vector[0], initialisation_vector[1]});
  }
}
void PoSyOptimiser::store_mean_smoothness(SurfelGraphNodePtr node, float smoothness) const {
  node->data()->set_posy_smoothness(smoothness);
}
