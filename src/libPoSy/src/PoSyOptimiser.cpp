//
// Created by Dave Durbin on 18/5/20.
//

#include "PoSyOptimiser.h"

#include <spdlog/spdlog.h>
#include <PoSy.h>
#include <Geom/Geom.h>
#include <Eigen/Geometry>
#include <Surfel/SurfelGraph.h>

PoSyOptimiser::PoSyOptimiser(const Properties &properties, std::default_random_engine &rng)
    : NodeOptimiser{properties, rng} //
{
  m_rho = m_properties.getFloatProperty("rho");

  setup_termination_criteria(
      "posy-termination-criteria",
      "posy-term-crit-relative-smoothness",
      "posy-term-crit-absolute-smoothness",
      "posy-term-crit-max-iterations");

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
PoSyOptimiser::compute_smoothness_in_frame(
    const SurfelGraph::Edge &edge,
    unsigned int frame_idx) const {

  auto surfel = edge.from()->data();
  auto surfel_lattice_offset = surfel->reference_lattice_offset();
  auto nbr_surfel = edge.to()->data();
  auto nbr_surfel_lattice_offset = nbr_surfel->reference_lattice_offset();

  Eigen::Vector3f vertex, normal, tangent;
  surfel->get_vertex_tangent_normal_for_frame(frame_idx, vertex, tangent, normal);

  Eigen::Vector3f nbr_vertex, nbr_normal, nbr_tangent;
  nbr_surfel->get_vertex_tangent_normal_for_frame(frame_idx, nbr_vertex, nbr_tangent, nbr_normal);

  unsigned short k_ij, k_ji;
  if (surfel->id() < nbr_surfel->id()) {
    k_ij = edge.data()->k_low();
    k_ji = edge.data()->k_high();
  } else {
    k_ij = edge.data()->k_high();
    k_ji = edge.data()->k_low();
  }
  // Orient tangents appropriately for frame based on k_ij and k_ji
  const auto &oriented_tangent = vector_by_rotating_around_n(tangent, normal, k_ij);
  const auto &nbr_oriented_tangent = vector_by_rotating_around_n(nbr_tangent, nbr_normal, k_ji);

  // Compute orth tangents
  const auto &orth_tangent = normal.cross(oriented_tangent);
  const auto &nbr_orth_tangent = nbr_normal.cross(nbr_oriented_tangent);

  // Compute lattice points for this node and neighbour
  Eigen::Vector3f nearest_lattice_point = vertex +
      surfel_lattice_offset[0] * oriented_tangent +
      surfel_lattice_offset[1] * orth_tangent;
  const auto nbr_nearest_lattice_point = nbr_vertex +
      nbr_surfel_lattice_offset[0] * nbr_oriented_tangent +
      nbr_surfel_lattice_offset[1] * nbr_orth_tangent;

  // Compute q_ij ... the midpoint on the intersection of the tangent planes
  std::vector<Eigen::Vector3f> Qi, Qj;
  const auto q = compute_qij(vertex, normal, nbr_vertex, nbr_normal);
  const auto closest_points = compute_closest_points(
      nearest_lattice_point, oriented_tangent, orth_tangent,
      nbr_nearest_lattice_point, nbr_oriented_tangent, nbr_orth_tangent,
      q, m_rho,
      Qi, Qj);

  const auto cp_i = closest_points.first;
  auto cp_j = closest_points.second;

  // The smoothness is how much difference there is in the
  // agreement of the closest points to the lattice
  const auto delta = (cp_j - cp_i).squaredNorm();
  if (m_properties.getBooleanProperty("diagnose_dodgy_deltas")) {
    // Closest points should be at most m_rho  * / sqrt(2) apart
    static double MAX_D = (m_rho * m_rho * 0.5);
    if (delta >= MAX_D) {
      spdlog::warn("Unlikely looking closest distance {:4} for surfels {} and {}", delta,
                   surfel->id(), nbr_surfel->id());
      spdlog::warn("v_i = [{:3}, {:3}, {:3}]", vertex[0], vertex[1], vertex[2]);
      spdlog::warn("tan_i = [{:3}, {:3}, {:3}]", oriented_tangent[0], oriented_tangent[1], oriented_tangent[2]);
      spdlog::warn("otan_i = [{:3}, {:3}, {:3}]", orth_tangent[0], orth_tangent[1], orth_tangent[2]);
      spdlog::warn("n_i = [{:3f}, {:3f}, {:3f}]", normal[0], normal[1], normal[2]);
      spdlog::warn("v_j = [{:3}, {:3}, {:3}]", nbr_vertex[0], nbr_vertex[1], nbr_vertex[2]);
      spdlog::warn("tan_j = [{:3}, {:3}, {:3}]", nbr_tangent[0], nbr_tangent[1], nbr_tangent[2]);
      spdlog::warn("otan_j = [{:3}, {:3}, {:3}]", nbr_orth_tangent[0], nbr_orth_tangent[1],
                   nbr_orth_tangent[2]);
      spdlog::warn("n_j = [{:3f}, {:3f}, {:3f}]", nbr_normal[0], nbr_normal[1], nbr_normal[2]);
      spdlog::warn("q_ij = [{:3f}, {:3f}, {:3f}]", q[0], q[1], q[2]);
      spdlog::warn("cl_i = [{:3f}, {:3f}, {:3f}]", cp_i[0], cp_i[1], cp_i[2]);
      spdlog::warn("cl_j = [{:3f}, {:3f}, {:3f}]", cp_j[0], cp_j[1], cp_j[2]);
      spdlog::warn(
          "Qi = [{:3f}, {:3f}, {:3f};\n\t\t\t\t\t\t{:3f}, {:3f}, {:3f};\n\t\t\t\t\t\t{:3f}, {:3f}, {:3f};\n\t\t\t\t\t\t{:3f}, {:3f}, {:3f};\n\t\t\t\t\t\t{:3f}, {:3f}, {:3f}]",
          Qi[0][0],
          Qi[0][1],
          Qi[0][2],
          Qi[1][0],
          Qi[1][1],
          Qi[1][2],
          Qi[3][0],
          Qi[3][1],
          Qi[3][2],
          Qi[2][0],
          Qi[2][1],
          Qi[2][2],
          Qi[0][0],
          Qi[0][1],
          Qi[0][2]
      );
      spdlog::warn(
          "Qj = [{:3f}, {:3f}, {:3f};\n\t\t\t\t\t\t{:3f}, {:3f}, {:3f};\n\t\t\t\t\t\t{:3f}, {:3f}, {:3f};\n\t\t\t\t\t\t{:3f}, {:3f}, {:3f};\n\t\t\t\t\t\t{:3f}, {:3f}, {:3f}]",
          Qj[0][0],
          Qj[0][1],
          Qj[0][2],
          Qj[1][0],
          Qj[1][1],
          Qj[1][2],
          Qj[3][0],
          Qj[3][1],
          Qj[3][2],
          Qj[2][0],
          Qj[2][1],
          Qj[2][2],
          Qj[0][0],
          Qj[0][1],
          Qj[0][2]
      );
      spdlog::warn("curr_lp = [{:3f}, {:3f}, {:3f}]",
                   nearest_lattice_point[0],
                   nearest_lattice_point[1],
                   nearest_lattice_point[2]);
      spdlog::warn("nbr_lp = [{:3f}, {:3f}, {:3f}]",
                   nbr_nearest_lattice_point[0],
                   nbr_nearest_lattice_point[1],
                   nbr_nearest_lattice_point[2]);
    }
  }
  return delta;
}

/**
 * Comparator that sorts two SurfelGRaphNodePtrs based on the smoothness
 * with the largest smoothness first.
 */
bool
PoSyOptimiser::compare_worst_first(const SurfelGraphNodePtr &l, const SurfelGraphNodePtr &r) const {
  return l->data()->posy_smoothness() > r->data()->posy_smoothness();
}

/**
 * Smooth the given node with it's neighbour in the specific frame
 * The input ref_lattice_offset is used.
 */
Eigen::Vector2f
PoSyOptimiser::smooth_node_in_frame(//
    unsigned int frame_idx,
    const std::shared_ptr<Surfel> &from_surfel,
    const Eigen::Vector2f &from_lattice_offset,
    float w_i,
    const std::shared_ptr<Surfel> &to_surfel,
    float w_j,
    unsigned short delta
) const {
  using namespace Eigen;

  // Compute the lattice offset 3D position in the given frame
  Vector3f from_vertex, from_tangent, from_normal;
  from_surfel->get_vertex_tangent_normal_for_frame(frame_idx, from_vertex, from_tangent, from_normal);
  Vector3f from_orth_tangent = from_normal.cross(from_tangent);
  Vector3f from_lattice_vertex = from_vertex +
      from_lattice_offset[0] * from_tangent +
      from_lattice_offset[1] * from_orth_tangent;

  // Compute the neighbour's lattice offset 3D position in the given frame
  Vector3f to_vertex, to_tangent, to_normal;
  to_surfel->get_vertex_tangent_normal_for_frame(frame_idx, to_vertex, to_tangent, to_normal);
  Vector2f to_lattice_offset = to_surfel->reference_lattice_offset();
  Vector3f to_orth_tangent = to_normal.cross(to_tangent);
  Vector3f to_lattice_vertex = to_vertex +
      to_lattice_offset[0] * to_tangent +
      to_lattice_offset[1] * to_orth_tangent;

  // Orient the 'to' cross field' with respect to the 'from' field
  Vector3f adjusted_to_tangent = vector_by_rotating_around_n(to_tangent, to_normal, delta);
  Vector3f adjusted_to_orth_tangent = to_normal.cross(adjusted_to_tangent);

  // Compute the midpoint
  const auto midpoint = compute_qij(from_vertex, from_normal, to_vertex, to_normal);
  const auto closest_points = compute_closest_points(
      from_lattice_vertex, from_tangent, from_orth_tangent,
      to_lattice_vertex, adjusted_to_tangent, adjusted_to_orth_tangent,
      midpoint, m_rho);

  // Compute the weighted mean closest point
  Vector3f new_lattice_vertex = (w_i * closest_points.first) + (w_j * closest_points.second);
  new_lattice_vertex = new_lattice_vertex / (w_i + w_j);

  // Push back to tangent plane. New Lattice Vertex is now where we think the nearest lattice
  // vertex ought to be
  new_lattice_vertex = project_vector_to_plane(new_lattice_vertex, from_normal, false);

  // This is A lattice point in the new world but not necessarily the closest
  // to the vertex. To make it be that we need to subtract the vertex then
  // then truncate
  // ????
  from_lattice_vertex = round_4(from_normal, from_tangent, new_lattice_vertex, from_vertex, m_rho);

  // Convert back to an offset
  const auto diff = from_lattice_vertex - from_vertex;
  Vector2f new_lattice_offset = {diff.dot(from_tangent), diff.dot(from_orth_tangent)};

  if ((std::fabsf(new_lattice_offset[0]) - 0.5 > 1e-5) ||
      (std::fabsf(new_lattice_offset[1]) - 0.5 > 1e-5)) {
    spdlog::error("new_lattice_offset is invalid ({},{})",
                  new_lattice_offset[0],
                  new_lattice_offset[1]);
  }
  return new_lattice_offset;
}

std::vector<unsigned int>
PoSyOptimiser::filter_frames_list(
    const std::shared_ptr<Surfel> &from_surfel,
    const std::shared_ptr<Surfel> &to_surfel,
    const std::vector<unsigned int> &frames,
    const std::string &filter_name
) {
  std::vector<unsigned int> filtered_frames;
  for (const auto f: frames) {
    filtered_frames.push_back(f);
  }
  return filtered_frames;
}

/**
 * Optimise this GraphNode by considering all neighbours and allowing them all to
 * 'push' this node slightly to an agreed common position.
 * For a given pair of neighbours we can optimise for
 * "Smallest error" frame only
 * "Largest error" frame only
 * Always first frame
 * All frames
 * Any frame at random.
 * @param node
 */
void
PoSyOptimiser::optimise_node(const SurfelGraphNodePtr &node) {
  using namespace Eigen;

  const auto &curr_surfel = node->data();
  Vector2f new_lattice_offset = curr_surfel->reference_lattice_offset();

  // Smooth with each neighbour
  float weight_sum = 0.0;

  // Get all of this node's neighbours
  const auto neighbours = m_surfel_graph->neighbours(node);
  for (const auto &nbr_node: neighbours) {
    const auto &nbr_surfel = nbr_node->data();

    const auto &edge = m_surfel_graph->edge(node, nbr_node);

    // Get common frames for this neighbour
    auto frames_to_smooth = get_common_frames(curr_surfel, nbr_surfel);

    // If we're filtering frames and there's more than one, generate a list of frames to
    // smooth across.
    if (m_properties.hasProperty("posy-smooth-frame-filter") && frames_to_smooth.size() > 1) {
      frames_to_smooth = filter_frames_list(curr_surfel,
                                            nbr_surfel,
                                            frames_to_smooth,
                                            m_properties.getProperty("posy-smooth-frame-filter"));
    }

    Vector2i t_ij, t_ji;

    // For each frame in this list, smooth in that frame
    const auto delta = get_delta(m_surfel_graph, node, nbr_node);
    float w_ji = 1.0f;
    for (auto frame_idx: frames_to_smooth) {

      new_lattice_offset = smooth_node_in_frame(//
          frame_idx,
          curr_surfel, new_lattice_offset, weight_sum,
          nbr_surfel, w_ji, delta);
      weight_sum += w_ji;
    }
  }

  // Store the new offset
  curr_surfel->set_reference_lattice_offset(new_lattice_offset);
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

void PoSyOptimiser::ended_optimisation() {
  using namespace Eigen;
  using namespace std;

  int good_edges = 0;
  int bad_edges = 0;

  // Label graph edges - only works with meshes right now
  // TODO: Fix this to handle edges that don't occur in every frame
  for (const auto &edge: m_surfel_graph->edges()) {
    const auto &s1 = edge.from()->data();
    const auto &s2 = edge.to()->data();
    unsigned short delta = get_delta(m_surfel_graph, edge.from(), edge.to());

    spdlog::warn( "Edge from {} --> {}",
                  s1->id(), s2->id());

    for (int frame_idx = 0; frame_idx < m_num_frames; ++frame_idx) {
      Vector3f vertex, tangent, normal, orth_tangent;
      s1->get_vertex_tangent_normal_for_frame(frame_idx, vertex, tangent, normal);
      orth_tangent = normal.cross(tangent);
      Vector2f vertex_lattice_offset = s1->reference_lattice_offset();
      Vector3f vertex_lattice = vertex
          + (tangent * vertex_lattice_offset[0])
          + (orth_tangent * vertex_lattice_offset[1]);

      Vector3f nbr_vertex, nbr_tangent, nbr_normal, nbr_orth_tangent;
      s2->get_vertex_tangent_normal_for_frame(frame_idx, nbr_vertex, nbr_tangent, nbr_normal);
      nbr_orth_tangent = nbr_normal.cross(nbr_tangent);
      Vector2f nbr_vertex_lattice_offset = s2->reference_lattice_offset();
      Vector3f nbr_vertex_lattice = nbr_vertex
          + (nbr_tangent * nbr_vertex_lattice_offset[0])
          + (nbr_orth_tangent * nbr_vertex_lattice_offset[1]);

      Vector3f adjusted_nbr_tangent = vector_by_rotating_around_n(nbr_tangent, nbr_normal, delta);
      Vector3f adjusted_nbr_orth_tangent = nbr_normal.cross(adjusted_nbr_tangent);

      const auto midpoint = compute_qij(vertex, normal, nbr_vertex, nbr_normal);

      // Compute t_ij and t_ji for each edge
      const auto t = compute_tij_pair(vertex_lattice,
                       tangent,
                       orth_tangent,
                       nbr_vertex_lattice,
                       adjusted_nbr_tangent,
                       adjusted_nbr_orth_tangent,
                       midpoint,
                       m_rho);

      spdlog::warn( "  f: {}   t_ij ({}, {})   t_ji ({}, {})",
                    frame_idx, t.first[0], t.first[1],
                    t.second[0], t.second[1]);
      set_t(m_surfel_graph, edge.from(), t.first, edge.to(),t.second );
    }
  }
}

