//
// Created by Dave Durbin on 18/5/20.
//

#include "PoSyOptimiser.h"
#include "PoSy.h"
#include <utility>
#include <vector>
#include <RoSy/RoSy.h>
#include <Eigen/core>
#include <Eigen/Geometry>

static const char *SSA_SELECT_ALL_IN_RANDOM_ORDER = "select-all-in-random-order";
static const char *SSA_SELECT_WORST_100 = "select-worst-100";

// ========
/**
 * Construct a PoSyOptimiser.
 * @param properties Parameters for the optimiser.
 */
PoSyOptimiser::PoSyOptimiser(Properties properties) : AbstractOptimiser(std::move(properties)) {
    m_rho = m_properties.getFloatProperty("rho");
    m_convergence_threshold = m_properties.getFloatProperty("convergence-threshold");
    auto ssa = m_properties.getProperty("surfel-selection-algorithm");
    if (ssa == SSA_SELECT_ALL_IN_RANDOM_ORDER) {
        m_node_selection_function = bind(&PoSyOptimiser::ssa_select_all_in_random_order, this);
    } else if (ssa == SSA_SELECT_WORST_100) {
        m_node_selection_function = bind(&PoSyOptimiser::ssa_select_worst_100, this);
    } else {
        throw std::runtime_error("Unknown surfel selection algorithm " + ssa);
    }
}

PoSyOptimiser::~PoSyOptimiser() = default;


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
    for (const auto &n : m_surfel_graph.nodes()) {
        selected_nodes.push_back(n);
    }
    stable_sort(begin(selected_nodes), end(selected_nodes),
                [this](const SurfelGraphNodePtr &s1, const SurfelGraphNodePtr &s2) {
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


    size_t total_common_frames = 0;
    const auto &this_surfel_ptr = node->data();
    float weight = 0.0f;
    auto new_lattice_offset = this_surfel_ptr->closest_mesh_vertex_offset;
    // Get all neighbours of this Surfel
    Vector2f nudge = Vector2f::Zero();
    for (const auto &surfel_ptr_neighbour : m_surfel_graph.neighbours(node)) {
        const auto that_surfel_ptr = surfel_ptr_neighbour->data();

        // For this neighbour, consider it over all common frames.
        const auto common_frames = find_common_frames_for_surfels(this_surfel_ptr, that_surfel_ptr);
        total_common_frames += common_frames.size();

        // For each common frame, push surfel and neighbour into that frame
        for (auto const &frame_pair : common_frames) {
            const auto surfel_to_frame = frame_pair.first.get().transform;
            const auto surfel_tan_in_frame = surfel_to_frame * this_surfel_ptr->tangent;
            const auto surfel_normal_in_frame = frame_pair.first.get().normal;

            const auto neighbour_to_frame = frame_pair.second.get().transform;
            const auto neighbour_tan_in_frame = neighbour_to_frame * that_surfel_ptr->tangent;
            const auto neighbour_normal_in_frame = frame_pair.second.get().normal;

            // TODO(dave.d): We don't want to compute this every time. It should have been stabilised after computing
            // the orientation field and we should store it in the graph on edges.
            const auto best_pair = best_rosy_vector_pair(
                    surfel_tan_in_frame, surfel_normal_in_frame,
                    neighbour_tan_in_frame, neighbour_normal_in_frame );

            const auto distortion = compute_distortion(
                    frame_pair.first.get().position,
                    surfel_tan_in_frame,
                    surfel_normal_in_frame.cross(surfel_tan_in_frame),
                    new_lattice_offset,

                    frame_pair.second.get().position,
                    neighbour_tan_in_frame,
                    neighbour_normal_in_frame.cross(neighbour_tan_in_frame),
                    that_surfel_ptr->closest_mesh_vertex_offset
            );
            nudge = (weight * nudge + distortion) / (weight + 1.0f);
            weight += 1.0f;

            new_lattice_offset = this_surfel_ptr->closest_mesh_vertex_offset + nudge;
        }
    }
    this_surfel_ptr->closest_mesh_vertex_offset = new_lattice_offset;
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
        const Eigen::Vector3f &position1, const Eigen::Vector3f &tangent1, const Eigen::Vector3f &normal1, const Eigen::Vector2f &uv1,
        const Eigen::Vector3f &position2, const Eigen::Vector3f &tangent2, const Eigen::Vector3f &normal2, const Eigen::Vector2f &uv2) const {
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
        const Eigen::Vector3f &o_prime1,
        const Eigen::Vector2f &uv1,

        const Eigen::Vector3f &surfel_position2,
        const Eigen::Vector3f &o2,
        const Eigen::Vector3f &o_prime2,
        const Eigen::Vector2f &uv2
    ) const {
    using namespace std;
    using namespace Eigen;

    const auto lattice_vertex1 = surfel_position1 + (o1 * uv1.x()) + (o_prime1 * uv1.y());
    const auto l1 = compute_local_lattice_vertices(lattice_vertex1, o1, o_prime1, m_rho);

    const auto lattice_vertex2 = surfel_position2 + (o2 * uv2.x()) + (o_prime2 * uv2.y());
    const auto l2 = compute_local_lattice_vertices(lattice_vertex2, o2, o_prime2, m_rho);
    // best_idx_a, best_idx_b, points_a.at(best_idx_a), points_b.at(best_idx_b), min_dist_squared
    const auto tuple = closest_points(l1, l2);

    // Actual distance between lattice points in o1 and o_prime1 dimensions
    const auto actual_inter_vertex_vector = get<3>(tuple) - get<2>(tuple);
    const auto dist_o1 = actual_inter_vertex_vector.dot(o1);
    const auto dist_o_prime1 = actual_inter_vertex_vector.dot(o_prime1);

    // We would hope that these are _exact_ multiples of rho. The distortion is the amount by which they differ
    // from an exact multiple of rho.
    const auto remdr_o = fmod(dist_o1, m_rho);
    const auto remdr_o_prime = fmod(dist_o_prime1, m_rho);

    return Vector2f(remdr_o, remdr_o_prime);
}
