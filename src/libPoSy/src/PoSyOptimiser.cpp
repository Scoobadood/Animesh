//
// Created by Dave Durbin on 18/5/20.
//

#include "PoSyOptimiser.h"
#include "PoSy.h"
#include <utility>
#include <vector>
#include <RoSy/RoSy.h>
#include <Optimise/SurfelSelectionAlgorithm.h>
#include <Eigen/Core>
#include <Eigen/Geometry>

/**
 * Construct a PoSyOptimiser.
 * @param properties Parameters for the optimiser.
 */
PoSyOptimiser::PoSyOptimiser(Properties properties) : AbstractOptimiser(std::move(properties)) {
    m_rho = m_properties.getFloatProperty("rho");
    m_convergence_threshold = m_properties.getFloatProperty("convergence-threshold");
    m_node_selection_function = extractSsa(m_properties);
}

PoSyOptimiser::~PoSyOptimiser() = default;

std::function<std::vector<SurfelGraphNodePtr>(const AbstractOptimiser &)>
PoSyOptimiser::extractSsa(const Properties &properties) {
    auto ssa = with_name(properties.getProperty("surfel-selection-algorithm"));
    switch (ssa) {
        case SSA_SELECT_ALL_IN_RANDOM_ORDER:
            return std::bind(&PoSyOptimiser::ssa_select_all_in_random_order, this);

        case SSA_SELECT_WORST_100:
            return std::bind(&PoSyOptimiser::ssa_select_worst_100, this);
    }
}

/**
 * Entered the begin optimisation phase. Prepare by constructing cached records of important data that will
 * be used later to optimise.
 */
void PoSyOptimiser::optimisation_began() {
}

void PoSyOptimiser::optimisation_ended() {

}

/**
 * Surfel selection model 2: Select worst 100 smoothness scores
 */
std::vector<SurfelGraphNodePtr>
PoSyOptimiser::ssa_select_worst_100() {
    using namespace std;

    vector<SurfelGraphNodePtr> selected_nodes;
    for (const auto &n : m_surfel_graph->nodes()) {
        selected_nodes.push_back(n);
    }
    stable_sort(begin(selected_nodes), end(selected_nodes),
                [](const SurfelGraphNodePtr &s1, const SurfelGraphNodePtr &s2) {
                    return s1->data()->posy_smoothness > s2->data()->posy_smoothness;
                });

    selected_nodes.resize(100);
    return selected_nodes;
}

/**
 * Optimise this GraphNode by considering all neighbours and allowing them all to
 * 'push' this node slightly to an agreed common position.
 * @param node
 */
void PoSyOptimiser::optimise_node(const SurfelGraphNodePtr &node) {
    using namespace Eigen;

    const auto &this_surfel_ptr = node->data();
    auto new_mesh_vertex_offset = this_surfel_ptr->closest_mesh_vertex_offset;

    Vector2f nudge = Vector2f::Zero();
    float weight = 0.0f;

    // For each neighbour in the graph
    const auto this_surfel_neighbour_nodes = m_surfel_graph->neighbours(node);

    // For each frame this surfel is in
    for (const auto frame_index : this_surfel_ptr->frames) {

        // For each neighbour
        for (const auto &neighbour_node : this_surfel_neighbour_nodes) {

            // If the neighbour's not in the frame, skip
            if (!neighbour_node->data()->is_in_frame(frame_index)) {
                continue;
            }

            Eigen::Vector3f this_position, this_tangent, this_normal;
            this_surfel_ptr->get_position_tangent_normal_for_frame(frame_index, this_position, this_tangent,
                                                                   this_normal);

            Eigen::Vector3f that_position, that_tangent, that_normal;
            neighbour_node->data()->get_position_tangent_normal_for_frame(frame_index, that_position, that_tangent,
                                                                          that_normal);

            // TODO(dave.d): We don't want to compute this every time. It should have been stabilised after computing
            // the orientation field and we should store it in the graph on edges.
            best_rosy_vector_pair(
                    this_tangent, this_normal,
                    that_tangent, that_normal);

            const auto distortion = compute_distortion(
                    this_position,
                    this_tangent,
                    this_normal.cross(this_tangent),
                    new_mesh_vertex_offset,

                    that_position,
                    that_tangent,
                    that_normal.cross(that_tangent),
                    neighbour_node->data()->closest_mesh_vertex_offset
            );

            nudge = (weight * nudge + distortion) / (weight + 1.0f);
            weight += 1.0f;

            new_mesh_vertex_offset = this_surfel_ptr->closest_mesh_vertex_offset + nudge;
        } // End of neighbours
    } // End of frames
    this_surfel_ptr->closest_mesh_vertex_offset = new_mesh_vertex_offset;
}

/**
 * Compute the distortion of the u,v field between one surfel and another.
 *
 * @param normal1
 * @param tangent1
 * @param position1
 * @param normal2
 * @param tangent2
 * @param position2
 * @return
 */
float
PoSyOptimiser::compute_smoothness(
        const Eigen::Vector3f &position1, const Eigen::Vector3f &tangent1, const Eigen::Vector3f &normal1,
        const Eigen::Vector2f &uv1,
        const Eigen::Vector3f &position2, const Eigen::Vector3f &tangent2, const Eigen::Vector3f &normal2,
        const Eigen::Vector2f &uv2) const {
    using namespace std;
    using namespace Eigen;

    auto distortion = compute_distortion(
            position1,
            tangent1,
            normal1.cross(tangent1),
            uv1,

            position2,
            tangent2,
            normal2.cross(tangent2),
            uv2);

    return (distortion[0] * distortion[0] + distortion[1] * distortion[1]);
}

/**
 * Compute the distortion of the u,v field between one surfel and another.
 */
Eigen::Vector2f
PoSyOptimiser::compute_distortion(
        const Eigen::Vector3f &surfel_position1,
        const Eigen::Vector3f &o1,
        const Eigen::Vector3f &o1_prime,
        const Eigen::Vector2f &uv1,

        const Eigen::Vector3f &surfel_position2,
        const Eigen::Vector3f &o2,
        const Eigen::Vector3f &o2_prime,
        const Eigen::Vector2f &uv2
) const {
    using namespace std;
    using namespace Eigen;

    const auto lattice_vertex1 = surfel_position1 + (o1 * uv1.x()) + (o1_prime * uv1.y());
    const auto l1 = compute_local_lattice_vertices(lattice_vertex1, o1, o1_prime, m_rho);

    const auto lattice_vertex2 = surfel_position2 + (o2 * uv2.x()) + (o2_prime * uv2.y());
    const auto l2 = compute_local_lattice_vertices(lattice_vertex2, o2, o2_prime, m_rho);

    // following returns (best_idx_a, best_idx_b, points_a.at(best_idx_a), points_b.at(best_idx_b), min_dist_squared)
    const auto tuple = closest_points(l1, l2);

    // Actual distance between lattice points in o1 and o1_prime dimensions
    const auto actual_inter_vertex_vector = get<3>(tuple) - get<2>(tuple);
    const auto dist_o1 = actual_inter_vertex_vector.dot(o1);
    const auto dist_o1_prime = actual_inter_vertex_vector.dot(o1_prime);

    // We would hope that these are _exact_ multiples of rho. The distortion is the amount by which they differ
    // from an exact multiple of rho.
    const auto remdr_o = std::fmodf(dist_o1, m_rho);
    const auto remdr_o_prime = std::fmodf(dist_o1_prime, m_rho);

    // This is the distortion
    return {remdr_o, remdr_o_prime};
}
