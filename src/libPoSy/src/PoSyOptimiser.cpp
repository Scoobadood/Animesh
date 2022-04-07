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
      spdlog::warn("tan_j = [{:3}, {:3}, {:3}]",
                   nbr_default_tangent[0],
                   nbr_default_tangent[1],
                   nbr_default_tangent[2]);
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

Eigen::Vector3f
position_floor_4(const Eigen::Vector3f &lattice_origin, // Origin
                 const Eigen::Vector3f &tangent, // Rep tan
                 const Eigen::Vector3f &normal, // Normal
                 const Eigen::Vector3f &midpoint, // Point
                 float scale, float inv_scale) {
  using namespace Eigen;

  Vector3f orth_tangent = normal.cross(tangent);
  Vector3f d = midpoint - lattice_origin;
  // Computes the 'bottom left' lattice point closest to midpoint
  return lattice_origin +
      tangent * std::floor(tangent.dot(d) * inv_scale) * scale +
      orth_tangent * std::floor(orth_tangent.dot(d) * inv_scale) * scale;
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
  Vector2f curr_lattice_offset = curr_surfel->reference_lattice_offset();
  posy_logger->info("  starting lattice offset (default) is ({:.3f}, {:.3f})",
                    curr_lattice_offset[0], curr_lattice_offset[1]);

  // For each frame
  posy_logger->info("  surfel is present in {} frames", curr_surfel->frames().size());
  for (const auto frame_idx: curr_surfel->frames()) {
    posy_logger->info("  smoothing frame {}", frame_idx);

    // Compute the lattice offset 3D position in the given frame
    Vector3f curr_surfel_pos, curr_surfel_tangent, curr_surfel_normal;
    curr_surfel->get_vertex_tangent_normal_for_frame(frame_idx,
                                                     curr_surfel_pos,
                                                     curr_surfel_tangent,
                                                     curr_surfel_normal);
    const auto curr_surfel_orth_tangent = curr_surfel_normal.cross(curr_surfel_tangent);
    Vector3f working_clp = curr_surfel_pos +
        curr_lattice_offset[0] * curr_surfel_tangent +
        curr_lattice_offset[1] * curr_surfel_orth_tangent;
    posy_logger->info("    CLP starts at ({:3f} {:3f} {:3f})", working_clp[0], working_clp[1], working_clp[2]);

    // Get the neighbours of this surfel in this frame
    const auto &neighbours = get_node_neighbours_in_frame(AbstractOptimiser::m_surfel_graph, node, frame_idx);
    posy_logger->info("    smoothing with {} neighbours", neighbours.size());

    float sum_w = 0.0;
    for (const auto &nbr_node: neighbours) {
      const auto &nbr_surfel = nbr_node->data();
      const auto &nbr_lattice_offset = nbr_surfel->reference_lattice_offset();

      // Compute the neighbour's lattice offset 3D position in the given frame
      Vector3f nbr_surfel_pos, nbr_surfel_tangent, nbr_surfel_normal;
      nbr_surfel->get_vertex_tangent_normal_for_frame(frame_idx, nbr_surfel_pos, nbr_surfel_tangent, nbr_surfel_normal);
      const auto nbr_surfel_orth_tangent = nbr_surfel_normal.cross(nbr_surfel_tangent);
      Vector3f nbr_surfel_clp = nbr_surfel_pos +
          nbr_lattice_offset[0] * nbr_surfel_tangent +
          nbr_lattice_offset[1] * nbr_surfel_orth_tangent;
      posy_logger->info("      Neighbour {} at ({:.3f} {:.3f} {:.3f}) thinks CLP is at ({:.3f} {:.3f} {:.3f})",
                        nbr_surfel->id(),
                        nbr_surfel_pos[0],
                        nbr_surfel_pos[1],
                        nbr_surfel_pos[2],
                        nbr_surfel_clp[0],
                        nbr_surfel_clp[1],
                        nbr_surfel_clp[2]);

      // Compute the point that minimizes the distance to vertices while being located in their respective tangent plane
      const auto midpoint = compute_qij(curr_surfel_pos, curr_surfel_normal, nbr_surfel_pos, nbr_surfel_normal);
      posy_logger->info("        midpoint=[{:.3f} {:.3f} {:.3f}];", midpoint[0], midpoint[1], midpoint[2]);

      // The midpoint is contained on both tangent planes. Find the lattice points on both planes
      // which are closest to each other - checks 4 points surrounding midpoint on both planes
      // Origin, reptan, normal, point
      auto curr_surfel_base =
          position_floor_4(working_clp, curr_surfel_tangent, curr_surfel_normal, midpoint, m_rho, 1.0f / m_rho);
      auto nbr_surfel_base =
          position_floor_4(nbr_surfel_clp, nbr_surfel_tangent, nbr_surfel_normal, midpoint, m_rho, 1.0f / m_rho);

      auto best_cost = std::numeric_limits<float>::infinity();
      int best_i = -1, best_j = -1;
      for (int i = 0; i < 4; ++i) {
        // Derive ,o0t (test)  sequentiall, the other bounds of 1_ij in first plane
        Vector3f o0t =
            curr_surfel_base + (curr_surfel_tangent * (i & 1) + curr_surfel_orth_tangent * ((i & 2) >> 1)) * m_rho;
        for (int j = 0; j < 4; ++j) {
          Vector3f o1t =
              nbr_surfel_base
                  + (nbr_surfel_tangent * (j & 1) + nbr_surfel_orth_tangent * ((j & 2) >> 1)) * m_rho;
          auto cost = (o0t - o1t).squaredNorm();
          if (cost < best_cost) {
            best_i = i;
            best_j = j;
            best_cost = cost;
          }
        }
      }

      // best_i and best_j are the closest vertices surrounding q_ij
      // we return the pair of closest points on the lattice according to both origins
      auto closest_points = std::make_pair(
          curr_surfel_base
              + (curr_surfel_tangent * (best_i & 1) + curr_surfel_orth_tangent * ((best_i & 2) >> 1)) * m_rho,
          nbr_surfel_base
              + (nbr_surfel_tangent * (best_j & 1) + nbr_surfel_orth_tangent * ((best_j & 2) >> 1)) * m_rho);

      {
        posy_logger->info("        curr_closest=[{:.3f} {:.3f} {:.3f}];",
                          closest_points.first[0], closest_points.first[1], closest_points.first[2]);
        posy_logger->info("        nbr_closest=[{:.3f} {:.3f} {:.3f}];",
                          closest_points.second[0], closest_points.second[1], closest_points.second[2]);
      }

      // Compute the weighted mean closest point
      float w_j = 1.0f;
      working_clp = ((sum_w * closest_points.first) + (w_j * closest_points.second));
      sum_w += w_j;
      working_clp /= sum_w;
      posy_logger->info("        updated working_clp=[{:.3f} {:.3f} {:.3f}];",
                        working_clp[0], working_clp[1], working_clp[2]);

      // new_lattice_vertex is not necessarily on the plane of the from tangents
      // We may need to correct for this later
      working_clp -= curr_surfel_normal.dot(working_clp - curr_surfel_pos) * curr_surfel_normal;

      // new_lattice_vertex is now a point that we'd like to assume is on the lattice.
      // If this defines the lattice, now find the closest lattice point to curr_surfel_pos
      working_clp = round_4(curr_surfel_normal, curr_surfel_tangent, working_clp, curr_surfel_pos, m_rho);
      posy_logger->info("        rounded and normalised working_clp=[{:.3f} {:.3f} {:.3f}];",
                        working_clp[0], working_clp[1], working_clp[2]);

      // Convert the new LP into a
    } // Next neighbour

    // Convert back to offset.
    auto clp_offset = working_clp - curr_surfel_pos;
    auto u = clp_offset.dot(curr_surfel_tangent);
    auto v = clp_offset.dot(curr_surfel_orth_tangent);
    posy_logger->info("        new_lattice_offset=[{:.3f} {:.3f}];", u, v);

    node->data()->set_reference_lattice_offset({u, v});
  } // Next frame
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

void
PoSyOptimiser::label_edge(SurfelGraph::Edge &edge) {
  // Frames in which the edge occurs are frames which feature both start and end nodes
  auto frames_for_edge = get_common_frames(edge.from()->data(), edge.to()->data());

  Eigen::Vector2i t_ij, t_ji;
  compute_label_for_edge(edge, frames_for_edge, t_ij, t_ji);
  set_t(m_surfel_graph, edge.from(), t_ij, edge.to(), t_ji);
}

void
PoSyOptimiser::compute_label_for_edge( //
    const SurfelGraph::Edge &edge, //
    const std::vector<unsigned int> &frames_for_edge, //
    Eigen::Vector2i &t_ij, //
    Eigen::Vector2i &t_ji) const //
{
  using namespace Eigen;

  if (frames_for_edge.size() == 1) {
    return;
  }

  throw std::runtime_error("multi-frame labeling not supported");
}

/*
 * Apply t_ij, t_ji labels to the edge to indicate the relative rotational frames of
 * In a single frame these are guaranteed to be consistent. When there are multiple frames,
 * it's possible that different frames have different preferences. In this case we must
 * pick the 'best' orientation. More details on what best means is below.
 *
 */
void
PoSyOptimiser::label_edges() {
  using namespace std;
  using namespace spdlog;

  auto posy_logger = spdlog::get("posy-optimiser");
  posy_logger->trace(">> label_edges()");
  for (auto &edge: m_surfel_graph->edges()) {
    label_edge(edge);
  }
  posy_logger->trace("<< label_edges()");
}

void PoSyOptimiser::ended_optimisation() {
  label_edges();
}
