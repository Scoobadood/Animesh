//
// Created by Dave Durbin on 18/5/20.
//

#include "PoSyOptimiser.h"

#include <spdlog/spdlog.h>
#include <PoSy.h>
#include <Eigen/Geometry>

PoSyOptimiser::PoSyOptimiser(const Properties &properties)
        : Optimiser{properties} //
{
    m_rho = m_properties.getFloatProperty("rho");

    setup_termination_criteria(
            "posy-termination-criteria",
            "posy-term-crit-relative-smoothness",
            "posy-term-crit-absolute-smoothness",
            "posy-term-crit-max-iterations");
}

void
PoSyOptimiser::trace_smoothing(const SurfelGraphPtr &surfel_graph) const {
    const auto old_level = spdlog::default_logger_raw()->level();
    spdlog::default_logger_raw()->set_level(spdlog::level::trace);
    spdlog::debug("Round completed.");
    for (const auto &n : surfel_graph->nodes()) {
        Eigen::Vector3f p, no, t, t1, v;
        if (n->data()->is_in_frame(0)) {
            n->data()->get_all_data_for_surfel_in_frame(0, v, t, t1, no, p);
            spdlog::trace("  p : ({:3f}, {:3f}, {:3f}),  d : ({:3f}, {:3f}),   s : {:3f}",
                          p[0], p[1], p[2],
                          n->data()->posy_correction()[0],
                          n->data()->posy_correction()[1],
                          n->data()->posy_smoothness()
            );
        }
    }
    spdlog::default_logger_raw()->set_level(old_level);
}

float
PoSyOptimiser::compute_node_smoothness_for_frame(const SurfelGraphNodePtr &node_ptr,
                                                 size_t frame_index,
                                                 unsigned int &num_neighbours) const {
    float frame_smoothness = 0.0f;

    Eigen::Vector3f vertex, reference_lattice_vertex, normal, tangent, orth_tangent;
    node_ptr->data()->get_all_data_for_surfel_in_frame(
            frame_index,
            vertex,
            tangent,
            orth_tangent,
            normal,
            reference_lattice_vertex
    );

    const auto neighbours_in_frame = get_node_neighbours_in_frame(node_ptr, frame_index);

    // For each neighbour...
    for (const auto &neighbour_node : neighbours_in_frame) {

        Eigen::Vector3f nbr_vertex, nbr_reference_lattice_vertex, nbr_normal, nbr_tangent, nbr_orth_tangent;
        neighbour_node->data()->get_all_data_for_surfel_in_frame(
                frame_index,
                nbr_vertex,
                nbr_tangent,
                nbr_orth_tangent,
                nbr_normal,
                nbr_reference_lattice_vertex);

        // Compute q_ij ... the midpoint on the intersection of the tangent planes
        const auto q_ij = compute_qij(vertex, normal, nbr_vertex, nbr_normal);
        const auto Q_ij = compute_lattice_neighbours(reference_lattice_vertex, q_ij, tangent, orth_tangent, m_rho);
        const auto Q_ji = compute_lattice_neighbours(nbr_reference_lattice_vertex, q_ij, nbr_tangent, nbr_orth_tangent,
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
                spdlog::warn("tan_j = [{:3}, {:3}, {:3}]", nbr_tangent[0], nbr_tangent[1], nbr_tangent[2]);
                spdlog::warn("otan_i = [{:3}, {:3}, {:3}]", orth_tangent[0], orth_tangent[1], orth_tangent[2]);
                spdlog::warn("otan_j = [{:3}, {:3}, {:3}]", nbr_orth_tangent[0], nbr_orth_tangent[1],
                             nbr_orth_tangent[2]);
                spdlog::warn("v_j = [{:3}, {:3}, {:3}]", nbr_vertex[0], nbr_vertex[1], nbr_vertex[2]);
                spdlog::warn("q_ij = [{:3f}, {:3f}, {:3f}]", q_ij[0], q_ij[1], q_ij[2]);
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
                spdlog::warn("n_i = [{:3f}, {:3f}, {:3f}]", normal[0], normal[1], normal[2]);
                spdlog::warn("n_j = [{:3f}, {:3f}, {:3f}]", nbr_normal[0], nbr_normal[1], nbr_normal[2]);
                spdlog::warn("cl_i = [{:3f}, {:3f}, {:3f}]", cp_i[0], cp_i[1], cp_i[2]);
                spdlog::warn("cl_j = [{:3f}, {:3f}, {:3f}]", cp_j[0], cp_j[1], cp_j[2]);

                // Do some checking
                spdlog::warn("n_i . (Q_i3 - Q_i1) is {:4}", normal.dot(Q_ij[3] - Q_ij[1]));
                spdlog::warn("n_j . (Q_j3 - Q_j1) is {:4}", normal.dot(Q_ji[3] - Q_ji[1]));
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

Eigen::Vector2f compute_ref_offset(
        const Eigen::Vector3f &new_lattice_point,
        const Eigen::Vector3f &vertex,
        const Eigen::Vector3f &tangent,
        const Eigen::Vector3f &orth_tangent) {
    const auto diff = new_lattice_point - vertex;
    return {
            diff.dot(tangent),
            diff.dot(orth_tangent)
    };
}


/**
 * Optimise this GraphNode by considering all neighbours and allowing them all to
 * 'push' this node slightly to an agreed common position.
 * @param node
 */
void
PoSyOptimiser::optimise_node(const SurfelGraphNodePtr &node) {
    using namespace Eigen;

    const auto &this_surfel_ptr = node->data();

    // For each frame this surfel is in...
    for (const auto frame_index : this_surfel_ptr->frames()) {
        Eigen::Vector3f vertex, reference_lattice_point, normal, tangent, orth_tangent;
        this_surfel_ptr->get_all_data_for_surfel_in_frame(
                frame_index,
                vertex,
                tangent,
                orth_tangent,
                normal,
                reference_lattice_point);

        float sum_w = 0.0f;
        Vector3f new_lattice_point = reference_lattice_point;

        auto neighbours_in_frame = get_node_neighbours_in_frame(node, frame_index);
        // Optionally randomise the order
        if(m_randomise_neighour_order) {
            std::shuffle(begin(neighbours_in_frame),
                         end(neighbours_in_frame),
                         m_random_engine);
        }
        // For each neighbour j ...
        for (const auto &neighbour_node : neighbours_in_frame) {
            Eigen::Vector3f nbr_vertex, nbr_lattice_point, nbr_normal, nbr_tangent, nbr_orth_tangent;
            neighbour_node->data()->get_all_data_for_surfel_in_frame(
                    frame_index,
                    nbr_vertex,
                    nbr_tangent,
                    nbr_orth_tangent,
                    nbr_normal,
                    nbr_lattice_point);

            float w_ij = 1.0f;

            // Compute q_ij and thus Q_ij and Q_ji
            const auto q_ij = compute_qij(vertex, normal, nbr_vertex, nbr_normal);
            const auto Q_ij = compute_lattice_neighbours(new_lattice_point, q_ij, tangent, orth_tangent, m_rho);
            const auto Q_ji = compute_lattice_neighbours(nbr_lattice_point, q_ij, nbr_tangent, nbr_orth_tangent, m_rho);
            const auto closest_points = find_closest_points(Q_ij, Q_ji);

            new_lattice_point = sum_w * closest_points.first + w_ij * closest_points.second;
            sum_w += w_ij;
            new_lattice_point = new_lattice_point * (1.0f / sum_w);
            new_lattice_point -= normal.dot(new_lattice_point - vertex) * normal; // Back to plane
        } // End of neighbours
        new_lattice_point = round_4(normal, tangent, new_lattice_point, vertex, m_rho);
        const auto ref_offset = compute_ref_offset(new_lattice_point, vertex, tangent, orth_tangent);
        this_surfel_ptr->set_reference_lattice_offset(ref_offset);
    } // End of frames
}