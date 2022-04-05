//
// Created by Dave Durbin on 18/5/20.
//

#include "PoSyOptimiser.h"

#include <spdlog/spdlog.h>
#include <PoSy.h>
#include <Geom/Geom.h>
#include <Eigen/Geometry>
#include <Surfel/SurfelGraph.h>
#include <spdlog/sinks/basic_file_sink.h>

PoSyOptimiser::PoSyOptimiser( //
    const Properties &properties, //
    std::default_random_engine &rng //
) : NodeOptimiser{properties, rng} //
{
  m_rho = m_properties.getFloatProperty("rho");

  setup_termination_criteria(
      "posy-termination-criteria",
      "posy-term-crit-relative-smoothness",
      "posy-term-crit-absolute-smoothness",
      "posy-term-crit-max-iterations");

  setup_ssa();

  try {
    auto logger = spdlog::basic_logger_mt("posy-optimiser", "logs/posy-optimiser-trace.txt");
    logger->set_pattern("[posy optimiser] [%^%l%$] %v");
  }
  catch (const spdlog::spdlog_ex &ex) {
    std::cout << "Log init failed: " << ex.what() << std::endl;
  }

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

  Eigen::Vector3f vertex, normal, default_tangent;
  surfel->get_vertex_tangent_normal_for_frame(frame_idx, vertex, default_tangent, normal);

  Eigen::Vector3f nbr_vertex, nbr_normal, nbr_default_tangent;
  nbr_surfel->get_vertex_tangent_normal_for_frame(frame_idx, nbr_vertex, nbr_default_tangent, nbr_normal);

  unsigned short k_ij, k_ji;
  if (surfel->id() < nbr_surfel->id()) {
    k_ij = edge.data()->k_low();
    k_ji = edge.data()->k_high();
  } else {
    k_ij = edge.data()->k_high();
    k_ji = edge.data()->k_low();
  }
  // Orient tangents appropriately for frame based on k_ij and k_ji
  const auto &oriented_tangent = vector_by_rotating_around_n(default_tangent, normal, k_ij);
  const auto &nbr_oriented_tangent = vector_by_rotating_around_n(nbr_default_tangent, nbr_normal, k_ji);

  // Compute orth tangents
  const auto &orth_tangent = normal.cross(oriented_tangent);
  const auto &nbr_orth_tangent = nbr_normal.cross(nbr_oriented_tangent);

  // Compute lattice points for this node and neighbour (use defaults
  const auto nearest_lattice_point = vertex +
      surfel_lattice_offset[0] * default_tangent +
      surfel_lattice_offset[1] * normal.cross(default_tangent);
  const auto nbr_nearest_lattice_point = nbr_vertex +
      nbr_surfel_lattice_offset[0] * nbr_default_tangent +
      nbr_surfel_lattice_offset[1] * nbr_normal.cross(nbr_default_tangent);

  // Compute q_ij ... the midpoint on the intersection of the default_tangent planes

  const auto q = compute_qij(vertex, normal, nbr_vertex, nbr_normal);
  const auto closest_points = compute_closest_points(
      nearest_lattice_point, oriented_tangent, orth_tangent,
      nbr_nearest_lattice_point, nbr_oriented_tangent, nbr_orth_tangent,
      q, m_rho);

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
      spdlog::warn("n_i = [{:.3f}, {:.3f}, {:.3f}]", normal[0], normal[1], normal[2]);
      spdlog::warn("v_j = [{:3}, {:3}, {:3}]", nbr_vertex[0], nbr_vertex[1], nbr_vertex[2]);
      spdlog::warn("tan_j = [{:3}, {:3}, {:3}]", nbr_default_tangent[0], nbr_default_tangent[1], nbr_default_tangent[2]);
      spdlog::warn("otan_j = [{:3}, {:3}, {:3}]", nbr_orth_tangent[0], nbr_orth_tangent[1],
                   nbr_orth_tangent[2]);
      spdlog::warn("n_j = [{:.3f}, {:.3f}, {:.3f}]", nbr_normal[0], nbr_normal[1], nbr_normal[2]);
      spdlog::warn("q_ij = [{:.3f}, {:.3f}, {:.3f}]", q[0], q[1], q[2]);
      spdlog::warn("cl_i = [{:.3f}, {:.3f}, {:.3f}]", cp_i[0], cp_i[1], cp_i[2]);
      spdlog::warn("cl_j = [{:.3f}, {:.3f}, {:.3f}]", cp_j[0], cp_j[1], cp_j[2]);

      spdlog::warn("curr_lp = [{:.3f}, {:.3f}, {:.3f}]",
                   nearest_lattice_point[0],
                   nearest_lattice_point[1],
                   nearest_lattice_point[2]);
      spdlog::warn("nbr_lp = [{:.3f}, {:.3f}, {:.3f}]",
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
    const std::shared_ptr<Surfel> &from_surfel,
    const Eigen::Vector2f &from_lattice_offset,
    const std::shared_ptr<Surfel> &to_surfel,
    const Eigen::Vector2f &to_lattice_offset,
    unsigned int frame_idx,
    unsigned short k_ij,
    float w_i,
    unsigned short k_ji,
    float w_j,
    Eigen::Vector2i &t_ij,
    Eigen::Vector2i &t_ji
) const {
  using namespace Eigen;

  auto posy_logger = spdlog::get("posy-optimiser");

  // Compute the lattice offset 3D position in the given frame
  Vector3f from_surfel_pos, default_from_tangent, from_normal;
  from_surfel->get_vertex_tangent_normal_for_frame(frame_idx, from_surfel_pos, default_from_tangent, from_normal);
  const auto default_from_orth_tangent =from_normal.cross(default_from_tangent);
  // Compute the CLP using default tangent and orth tangent.
  Vector3f from_clp = from_surfel_pos +
      from_lattice_offset[0] * default_from_tangent +
      from_lattice_offset[1] * default_from_orth_tangent;

  // Compute local axes
  const auto from_u = vector_by_rotating_around_n(default_from_tangent, from_normal, k_ij);
  const auto from_v = vector_by_rotating_around_n(default_from_orth_tangent, from_normal, k_ij);

  // Compute the neighbour's lattice offset 3D position in the given frame
  Vector3f to_surfel_pos, default_to_tangent, to_normal;
  to_surfel->get_vertex_tangent_normal_for_frame(frame_idx, to_surfel_pos, default_to_tangent, to_normal);
  const auto default_to_orth_tangent = to_normal.cross(default_to_tangent);
  Vector3f to_clp = to_surfel_pos +
      to_lattice_offset[0] * default_to_tangent +
      to_lattice_offset[1] * default_to_orth_tangent;

  const auto to_u = vector_by_rotating_around_n(default_to_tangent, to_normal, k_ji);
  const auto to_v = vector_by_rotating_around_n(default_to_orth_tangent, to_normal, k_ji);

  {
    posy_logger->info("      Frame {}.", frame_idx);
    posy_logger->info("        from_surfel_pos=[{:.3f} {:.3f} {:.3f}];",
                      from_surfel_pos[0],
                      from_surfel_pos[1],
                      from_surfel_pos[2]);
    posy_logger->info("        from_surfel_clp=[{:.3f} {:.3f} {:.3f}];",
                      from_clp[0],
                      from_clp[1],
                      from_clp[2]);
    posy_logger->info("        from_u_axis=[{:.3f} {:.3f} {:.3f}];", from_u[0], from_u[1], from_u[2]);
    posy_logger->info("        from_v_axis=[{:.3f} {:.3f} {:.3f}];", from_v[0], from_v[1], from_v[2]);

    posy_logger->info("        to_surfel_pos=[{:.3f} {:.3f} {:.3f}];",
                      to_surfel_pos[0],
                      to_surfel_pos[1],
                      to_surfel_pos[2]);
    posy_logger->info("        to_surfel_clp=[{:.3f} {:.3f} {:.3f}];",
                      to_clp[0],
                      to_clp[1],
                      to_clp[2]);
    posy_logger->info("        to_u_axis=[{:.3f} {:.3f} {:.3f}];", to_u[0], to_u[1], to_u[2]);
    posy_logger->info("        to_v_axis=[{:.3f} {:.3f} {:.3f}];", to_v[0], to_v[1], to_v[2]);
  }

  // Compute the point that minimizes the distance to vertices while being located in their respective tangent plane
  const auto midpoint = compute_qij(from_surfel_pos, from_normal, to_surfel_pos, to_normal);
  posy_logger->info("        midpoint=[{:.3f} {:.3f} {:.3f}];", midpoint[0], midpoint[1], midpoint[2]);

  // The midpoint is contained on both tangent planes. Find the lattice points on both planes
  // which are closest to each other - checks 4 points surrounding midpoint on both planes
  std::vector<Vector3f> i_vecs;
  std::vector<Vector3f> j_vecs;
  const auto closest_points = compute_closest_points(
      from_clp, from_u, from_v,
      to_clp, to_u, to_v,
      midpoint, m_rho);

  {
    posy_logger->info("        from_clp=[{:.3f} {:.3f} {:.3f}];",
                      closest_points.first[0], closest_points.first[1], closest_points.first[2]);
    posy_logger->info("        to_clp=[{:.3f} {:.3f} {:.3f}];",
                      closest_points.second[0], closest_points.second[1], closest_points.second[2]);
  }

  // Compute the weighted mean closest point
  Vector3f new_lattice_vertex = ((w_i * closest_points.first) + (w_j * closest_points.second))/ (w_i + w_j);
  posy_logger->info("        new_lattice_vertex=[{:.3f} {:.3f} {:.3f}];",
                    new_lattice_vertex[0], new_lattice_vertex[1], new_lattice_vertex[2]);

  // new_lattice_vertex is not necessarily on the plane of the from tangents
  // We may need to correct for this later

  // new_lattice_vertex is now a point that we'd like to assume is on the lattice.
  // If this defines the lattice, now find the closest lattice point to from_surfel_pos
  const auto new_from_clp = round_4(from_normal, from_u, new_lattice_vertex, from_surfel_pos, m_rho);
  posy_logger->info("        new_from_clp=[{:.3f} {:.3f} {:.3f}];",
                    new_from_clp[0], new_from_clp[1], new_from_clp[2]);

  // Convert this to an offset in default coords for surfel
  const auto diff = new_from_clp - from_surfel_pos;
  Vector2f new_from_lattice_offset = {diff.dot(default_from_tangent), diff.dot(default_from_orth_tangent)};
  posy_logger->info("        new_from_lattice_offset=[{:.3f} {:.3f}];",
                    new_from_lattice_offset[0],
                    new_from_lattice_offset[1]);

  // Set it before we finish this frame
  from_surfel->set_reference_lattice_offset(new_from_lattice_offset);

  const auto t_ij_pair = compute_tij_pair(
      new_from_clp, from_u, from_v,
      to_clp, to_u, to_v,
      midpoint, m_rho);
  t_ij = t_ij_pair.first;
  t_ji = t_ij_pair.second;
  posy_logger->info("        t_ij, t_ji : ({}, {}) , ({}, {});", t_ij[0], t_ij[1], t_ji[0], t_ji[1]);
  return new_from_lattice_offset;
}

std::vector<unsigned int>
PoSyOptimiser::filter_frames_list(
    const std::shared_ptr<Surfel> &from_surfel,
    const std::shared_ptr<Surfel> &to_surfel,
    const std::vector<unsigned int> &frames,
    const std::string &filter_name
) {
  std::vector<unsigned int> filtered_frames;
  filtered_frames.reserve(frames.size());
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

  auto posy_logger = spdlog::get("posy-optimiser");

  const auto &curr_surfel = node->data();
  posy_logger->info("Optimising surfel {}", curr_surfel->id());

  // Ref latt offset in the default space.
  Vector2f new_lattice_offset = curr_surfel->reference_lattice_offset();
  posy_logger->info("  starting lattice offset (default) is ({:.3f}, {:.3f})",
                    new_lattice_offset[0], new_lattice_offset[1]);

  // Get all of this node's neighbours
  const auto neighbours = m_surfel_graph->neighbours(node);
  posy_logger->info("  smoothing with {} neighbours", neighbours.size());

  // Smooth with each neighbour - we go by neighbour since the k_ij,k_ji pir are consistent by neighbour
  // and it saves recomputing.
  float sum_w = 0.0;
  for (const auto &nbr_node: neighbours) {
    const auto &nbr_surfel = nbr_node->data();
    const auto &nbr_lattice_offset = nbr_surfel->reference_lattice_offset();

    const auto &edge = m_surfel_graph->edge(node, nbr_node);

    // Get common frames for this neighbour
    const auto frames_to_smooth = get_common_frames(curr_surfel, nbr_surfel);
    const auto &k = get_k(m_surfel_graph, node, nbr_node);
    posy_logger->info("    smoothing with {} across {} frames with orientation ({},{})",
                      nbr_surfel->id(), //
                      frames_to_smooth.size(), //
                      k.first, k.second //
    );

    Vector2i t_ij, t_ji;
    // For each frame in this list, smooth in that frame
    float w_ji = 1.0f;
    for (auto frame_idx: frames_to_smooth) {
      new_lattice_offset = smooth_node_in_frame(//
          curr_surfel, new_lattice_offset,
          nbr_surfel, nbr_lattice_offset,
          frame_idx,
          k.first, sum_w,
          k.second, w_ji,
          t_ij, t_ji);
      sum_w += w_ji;
    }
    set_t(m_surfel_graph, node, t_ij, nbr_node, t_ji);
  }
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

void PoSyOptimiser::ended_optimisation() {}
