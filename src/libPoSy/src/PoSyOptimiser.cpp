//
// Created by Dave Durbin on 18/5/20.
//

#include "PoSyOptimiser.h"

#include <spdlog/spdlog.h>
#include <PoSy.h>
#include <Geom/Geom.h>
#include <Eigen/Geometry>
#include <Surfel/SurfelGraph.h>

PoSyOptimiser::PoSyOptimiser(const Properties &properties)
    : Optimiser{properties} //
{
  m_rho = m_properties.getFloatProperty("rho");

  setup_termination_criteria(
      "posy-termination-criteria",
      "posy-term-crit-relative-smoothness",
      "posy-term-crit-absolute-smoothness",
      "posy-term-crit-max-iterations");
  m_randomise_neighour_order = m_properties.getBooleanProperty("posy-randomise-neighbour-order");
}

void
PoSyOptimiser::trace_smoothing(const SurfelGraphPtr &surfel_graph) const {
  spdlog::trace("Round completed.");
  for (const auto &n : surfel_graph->nodes()) {
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
                                                 unsigned int &num_neighbours,
                                                 bool is_first_run) const {
  float frame_smoothness = 0.0f;

  const auto neighbours_in_frame = get_neighbours_of_node_in_frame(m_surfel_graph, node_ptr, frame_index, false);

  // For each neighbour...
  for (const auto &neighbour_node : neighbours_in_frame) {

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
    const auto &edge = m_surfel_graph->edge(node_ptr, neighbour_node);
    const auto k_ij = edge->k_ij(frame_index);
    const auto k_ji = edge->k_ji(frame_index);

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

/**
 * Optimise this GraphNode by considering all neighbours and allowing them all to
 * 'push' this node slightly to an agreed common position.
 * @param node
 */
void
PoSyOptimiser::optimise_node(const SurfelGraphNodePtr &node) {
  using namespace Eigen;

  const auto &curr_surfel = node->data();
  auto curr_lattice_offset = curr_surfel->reference_lattice_offset();

  // For each frame this surfel is in...
  for (const auto frame_index : curr_surfel->frames()) {

    // Get this surfels vertex, tangent, normal and nearest lattice point as they appear in frame
    Vector3f vertex, normal, tangent;
    curr_surfel->get_vertex_tangent_normal_for_frame(frame_index, vertex, tangent, normal);
    Vector3f otan = normal.cross(tangent);
    Vector3f curr_lattice_point = vertex +
        (tangent * curr_lattice_offset[0]) +
        (otan * curr_lattice_offset[1]);

          // Consider every neighbour in this frame
    float sum_w = 1.0f;
    auto neighbours_in_frame = get_neighbours_of_node_in_frame(m_surfel_graph, node,
                                                               frame_index, m_randomise_neighour_order);
    for (const auto &neighbour_node : neighbours_in_frame) {

        // Collect neighbour data
      Vector3f nbr_vertex, nbr_normal, nbr_tangent;
      neighbour_node->data()->get_vertex_tangent_normal_for_frame(frame_index, nbr_vertex, nbr_tangent,
                                                                  nbr_normal);
      auto nbr_lattice_offset = neighbour_node->data()->reference_lattice_offset();
      Vector3f nbr_lattice_point = nbr_vertex +
          (nbr_tangent * nbr_lattice_offset[0]) +
          ((nbr_normal.cross(nbr_tangent)) * nbr_lattice_offset[1]);

      // And edge data
      const auto &edge = m_surfel_graph->edge(node, neighbour_node);
      const auto k_ij = edge->k_ij(frame_index);
      const auto k_ji = edge->k_ji(frame_index);
      float w_ij = edge->weight();

      // Orient tangents appropriately for frame based on k_ij and k_ji
      const auto &aligned_tangent = vector_by_rotating_around_n(tangent, normal, k_ij);
      const auto &orth_tangent = normal.cross(aligned_tangent);
      const auto &nbr_aligned_tangent = vector_by_rotating_around_n(nbr_tangent, nbr_normal, k_ji);
      const auto &nbr_orth_tangent = nbr_normal.cross(nbr_aligned_tangent);

      if( curr_surfel->id() == "v_138" ) {
        spdlog::info("{}-->{}:: p_i =[{:0.3f}, {:0.3f}, {:0.3f}]",
                     node->data()->id(), neighbour_node->data()->id(),
                     vertex[0], vertex[1], vertex[2]);
        spdlog::info("            :: t_i =[{:0.3f}, {:0.3f}, {:0.3f}]",
                     tangent[0], tangent[1], tangent[2]);
        spdlog::info("            :: o_i =[{:0.3f}, {:0.3f}, {:0.3f}]",
                     otan[0], otan[1], otan[2]);
        spdlog::info("            :: u_i =[{:0.3f}, {:0.3f}, {:0.3f}]",
                     aligned_tangent[0], aligned_tangent[1], aligned_tangent[2]);
        spdlog::info("            :: v_i =[{:0.3f}, {:0.3f}, {:0.3f}]",
                     orth_tangent[0], orth_tangent[1], orth_tangent[2]);
        spdlog::info("            :: p_j =[{:0.3f}, {:0.3f}, {:0.3f}]",
                     nbr_vertex[0], nbr_vertex[1], nbr_vertex[2]);
        spdlog::info("            :: t_j =[{:0.3f}, {:0.3f}, {:0.3f}]",
                     nbr_tangent[0], nbr_tangent[1], nbr_tangent[2]);
        spdlog::info("            :: o_j =[{:0.3f}, {:0.3f}, {:0.3f}]",
                     nbr_normal.cross(nbr_tangent)[0], nbr_normal.cross(nbr_tangent)[1], nbr_normal.cross(nbr_tangent)[2]);
        spdlog::info("            :: u_j =[{:0.3f}, {:0.3f}, {:0.3f}]",
                     nbr_aligned_tangent[0], nbr_aligned_tangent[1], nbr_aligned_tangent[2]);
        spdlog::info("            :: v_j =[{:0.3f}, {:0.3f}, {:0.3f}]",
                     nbr_orth_tangent[0], nbr_orth_tangent[1], nbr_orth_tangent[2]);
        spdlog::info("            :: curr_lp =[{:0.3f}, {:0.3f}, {:0.3f}]",
                     curr_lattice_point[0], curr_lattice_point[1], curr_lattice_point[2]);
        spdlog::info("            :: nbr_lp =[{:0.3f}, {:0.3f}, {:0.3f}]",
                     nbr_lattice_point[0], nbr_lattice_point[1], nbr_lattice_point[2]);
      }
        // Compute q, the midpoint, and thus Q_ij and Q_ji and then the closest pair of these
      const auto q = compute_qij(vertex, normal, nbr_vertex, nbr_normal);
      const auto Q_ij = compute_lattice_neighbours(curr_lattice_point, q, aligned_tangent, orth_tangent, m_rho);
      const auto Q_ji = compute_lattice_neighbours(nbr_lattice_point, q, nbr_aligned_tangent, nbr_orth_tangent, m_rho);
      const auto closest_points = find_closest_points(Q_ij, Q_ji);

      // Compute t_ij and t_ji in oriented space!
      // TODO: Fix this bit. We're missing an important part of the calculation

      const auto dxyz_ij = (q - curr_lattice_point);
      const auto t_ij_0 = (int) round(dxyz_ij.dot(aligned_tangent));
      const auto t_ij_1 = (int) round(dxyz_ij.dot(orth_tangent));
      const auto dxyz_ji = (q - nbr_lattice_point);
      const auto t_ji_0 = (int) round(dxyz_ji.dot(nbr_aligned_tangent));
      const auto t_ji_1 = (int) round(dxyz_ji.dot(nbr_orth_tangent));
      edge->set_t_ij(frame_index, t_ij_0, t_ij_1);
      edge->set_t_ji(frame_index, t_ji_0, t_ji_1);

      // Compute the weighted mean closest point
      curr_lattice_point = sum_w * closest_points.first + w_ij * closest_points.second;
      sum_w += w_ij;
      curr_lattice_point = curr_lattice_point * (1.0f / sum_w);

      if(curr_surfel->id() == "v_138" ) {
        spdlog::info( "            :: q =[{}, {}, {}]",
                      q[0], q[1], q[2]);
        spdlog::info( "            :: Qij =[{}, {}, {};{} {} {}; {} {} {}; {} {} {}; {} {} {}]",
                      Q_ij[0][0], Q_ij[0][1], Q_ij[0][2],
                      Q_ij[1][0], Q_ij[1][1], Q_ij[1][2],
                      Q_ij[3][0], Q_ij[3][1], Q_ij[3][2],
                      Q_ij[2][0], Q_ij[2][1], Q_ij[2][2],
                      Q_ij[0][0], Q_ij[0][1], Q_ij[0][2]);
        spdlog::info( "            :: Qji =[{}, {}, {};{} {} {}; {} {} {}; {} {} {}; {} {} {}]",
                      Q_ji[0][0], Q_ji[0][1], Q_ji[0][2],
                      Q_ji[1][0], Q_ji[1][1], Q_ji[1][2],
                      Q_ji[3][0], Q_ji[3][1], Q_ji[3][2],
                      Q_ji[2][0], Q_ji[2][1], Q_ji[2][2],
                      Q_ji[0][0], Q_ji[0][1], Q_ji[0][2]);
        spdlog::info( "            :: cp_i =[{}, {}, {}]",
                      closest_points.first[0], closest_points.first[1], closest_points.first[2]);
        spdlog::info( "            :: cp_j =[{}, {}, {}]",
                      closest_points.second[0], closest_points.second[1], closest_points.second[2]);
        spdlog::info( "            :: t_ij=[{}, {}], t_ji =[{}, {}]",
                      t_ij_0, t_ij_1, t_ji_0, t_ji_1);
      }
    } // End of neighbours

    // Convert back to a reference lattice offset in the k_ij = 0 space
    curr_lattice_point = round_4(normal, tangent, curr_lattice_point, vertex, m_rho);
    const auto diff = curr_lattice_point - vertex;
    curr_lattice_offset = {diff.dot(tangent), diff.dot(otan)};
    if( curr_lattice_offset[0] > 0.5 || curr_lattice_offset[0] < -0.5 || curr_lattice_offset[1] > 0.5 || curr_lattice_offset[1] < -0.5) {
      int x = 9;
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
  for (auto &node : m_surfel_graph->nodes()) {
    node->data()->set_reference_lattice_offset({initialisation_vector[0], initialisation_vector[1]});
  }
}
void PoSyOptimiser::store_mean_smoothness(SurfelGraphNodePtr node, float smoothness) const {
  node->data()->set_posy_smoothness(smoothness);
}
