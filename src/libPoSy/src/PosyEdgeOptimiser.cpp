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
  const auto k = get_k(m_surfel_graph, edge.from(), edge.to());
  const auto k_ij = k.first;
  const auto k_ji = k.second;

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
