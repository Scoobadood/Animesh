//
// Created by Dave Durbin (Old) on 8/11/21.
//

#include "PosyEdgeOptimiser.h"
#include <Geom/Geom.h>
#include "PoSy.h"
#include <Eigen/Geometry>
#include <utility>

PosyEdgeOptimiser::PosyEdgeOptimiser(Properties properties, std::mt19937 &rng) //
    : EdgeOptimiser(std::move(properties), rng) //
    , m_rho{1.0} //
{
  m_rho = m_properties.getFloatProperty("rho");
}

void
PosyEdgeOptimiser::optimise_edge(const SurfelGraph::Edge &edge) {
  using namespace Eigen;

  const auto &from_surfel = edge.from()->data();
  const auto &to_surfel = edge.to()->data();

  Vector2f from_lattice_offset = from_surfel->reference_lattice_offset();
  Vector2f to_lattice_offset = to_surfel->reference_lattice_offset();

  auto common_frames = get_common_frames(from_surfel, to_surfel);

  // Either per frame, or one frame at ranomd or whatever frame selectionalgo we use.
  unsigned int frame_idx = common_frames[0];


  // Compute the current vertex 3D position of nearest lattice vertex in this frame
  Vector3f vertex, tangent, normal;
  from_surfel->get_vertex_tangent_normal_for_frame(frame_idx, vertex, tangent, normal);
  Vector3f orth_tangent = normal.cross(tangent);
  Vector3f from_lattice_vertex = vertex +
      from_lattice_offset[0] * tangent +
      from_lattice_offset[1] * orth_tangent;

  // Collect neighbour data
  Vector3f nbr_vertex, nbr_tangent, nbr_normal;
  to_surfel->get_vertex_tangent_normal_for_frame(frame_idx, nbr_vertex, nbr_tangent, nbr_normal);
  Vector3f nbr_orth_tangent = nbr_normal.cross(nbr_tangent);
  Vector3f nbr_lattice_vertex = nbr_vertex +
      to_lattice_offset[0] * nbr_tangent +
      to_lattice_offset[1] * nbr_orth_tangent;

  // Get edge data for the edge from this node to this neighbour
  unsigned short k_ij, k_ji;
  if (edge.from()->data()->id() < edge.to()->data()->id()) {
    k_ij = edge.data()->k_low();
    k_ji = edge.data()->k_high();
  } else {
    k_ij = edge.data()->k_high();
    k_ji = edge.data()->k_low();
  }


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
      from_lattice_vertex, edge_adjusted_tangent, edge_adjusted_orth_tangent,
      nbr_lattice_vertex, nbr_edge_adjusted_tangent, nbr_edge_adjusted_orth_tangent,
      midpoint, m_rho);

  const auto closest_points = compute_closest_points(
      from_lattice_vertex, edge_adjusted_tangent, edge_adjusted_orth_tangent,
      nbr_lattice_vertex, nbr_edge_adjusted_tangent, nbr_edge_adjusted_orth_tangent,
      midpoint, m_rho
  );

  set_t(m_surfel_graph, edge.from(), t_ij_pair.first, edge.to(), t_ij_pair.second);

  // Compute the weighted mean closest point
  Vector3f lattice_vertex = (0.5 * closest_points.first) + (0.5 * closest_points.second);

  // Push back to tangent plane of from surfel
  from_lattice_vertex = lattice_vertex - (normal.dot(lattice_vertex - vertex) * normal);
  nbr_lattice_vertex = lattice_vertex - (nbr_normal.dot(lattice_vertex - nbr_vertex) * nbr_normal);

  // Make sure this is actually the closest lattice vertex by doing the rounding thing
  from_lattice_vertex = round_4(normal, edge_adjusted_tangent, from_lattice_vertex, vertex, m_rho);
  nbr_lattice_vertex = round_4(nbr_normal, nbr_edge_adjusted_tangent, nbr_lattice_vertex, nbr_vertex, m_rho);

// Convert back to an offset at k=0
  const auto from_diff = from_lattice_vertex - vertex;
  const auto to_diff = nbr_lattice_vertex - nbr_vertex;
  from_lattice_offset = {from_diff.dot(tangent), from_diff.dot(orth_tangent)};
  if ((std::fabsf(from_lattice_offset[0]) - 0.5 > 1e-5) ||
      (std::fabsf(from_lattice_offset[1]) - 0.5 > 1e-5)) {
    spdlog::error("from_lattice_offset is too far away {},{}",
                  from_lattice_offset[0],
                  from_lattice_offset[1]);
  }
  from_surfel->set_reference_lattice_offset(from_lattice_offset);

  to_lattice_offset = {to_diff.dot(nbr_tangent), to_diff.dot(nbr_orth_tangent)};
  if ((std::fabsf(to_lattice_offset[0]) - 0.5 > 1e-5) ||
      (std::fabsf(to_lattice_offset[1]) - 0.5 > 1e-5)) {
    spdlog::error("to_lattice_offset is too far away {},{}",
                  to_lattice_offset[0],
                  to_lattice_offset[1]);
  }
  to_surfel->set_reference_lattice_offset(to_lattice_offset);
}


// ================== Should be abstracted away as its shared with PoSyOpitimiser
float
PosyEdgeOptimiser::compute_node_smoothness_for_frame(const SurfelGraphNodePtr &node_ptr,
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
    const auto q = compute_qij(vertex, normal, nbr_vertex, nbr_normal);

    // Compute q_ij ... the midpoint on the intersection of the tangent planes
    // -----
    const auto closest_points = compute_closest_points(
        nearest_lattice_point, oriented_tangent, orth_tangent,
        nbr_nearest_lattice_point, nbr_oriented_tangent, nbr_orth_tangent,
        q, m_rho
    );
    // ------

    const auto cp_i = closest_points.first;
    const auto cp_j = closest_points.second;

    // Compute the smoothness over this surfel in this frame and the neighbour in this frame.
    const auto delta = (cp_j - cp_i).squaredNorm();
    frame_smoothness += delta;
  }
  num_neighbours = neighbours_in_frame.size();
  return frame_smoothness;
}

void PosyEdgeOptimiser::store_mean_smoothness(SurfelGraphNodePtr node, float smoothness) const {
  node->data()->set_posy_smoothness(smoothness);
}
