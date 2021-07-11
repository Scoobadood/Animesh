#include "RoSy/RoSyOptimiser.h"

#include <RoSy/RoSy.h>
#include <Properties/Properties.h>
#include <Surfel/SurfelGraph.h>

#include <Eigen/Core>

/**
 * Construct one with the given properties.
 * @param properties
 */
RoSyOptimiser::RoSyOptimiser(const Properties &properties)
        : Optimiser(properties) //
{
    setup_termination_criteria(
            "rosy-termination-criteria",
            "rosy-term-crit-relative-smoothness",
            "rosy-term-crit-absolute-smoothness",
            "rosy-term-crit-max-iterations");

    m_damping_factor = m_properties.getFloatProperty("rosy-damping-factor");
    m_weight_for_error = m_properties.getBooleanProperty("rosy-weight-for-error");
    m_weight_for_error_steps = m_properties.getIntProperty("rosy-weight-for-error-steps");
    m_randomise_neighour_order = m_properties.getBooleanProperty("rosy-randomise-neighbour-order");

}

/**
 * @return The mean error per neighbour.
 */
float
RoSyOptimiser::compute_node_smoothness_for_frame(
        const SurfelGraphNodePtr &this_node,
        size_t frame_index,
        unsigned int &num_neighbours) const {

    using namespace Eigen;

    float frame_smoothness = 0.0f;

    const auto &this_surfel = this_node->data();
    spdlog::debug("Computing smoothness for node {} in frame {}", this_surfel->id(), frame_index);

    Vector3f vertex, normal, tangent;
    this_surfel->get_vertex_tangent_normal_for_frame(frame_index, vertex, tangent, normal);

    // For each neighbour in frame...
    const auto neighbours_in_frame = get_node_neighbours_in_frame(m_surfel_graph, this_node, frame_index);
    for (const auto &nbr_node : neighbours_in_frame) {
        const auto &nbr_surfel = nbr_node->data();

        Vector3f nbr_vertex, nbr_normal, nbr_tangent;
        nbr_surfel->get_vertex_tangent_normal_for_frame(frame_index, nbr_vertex, nbr_tangent, nbr_normal);

        // Get the edge between this pair
        const auto edge = m_surfel_graph->edge(this_node, nbr_node);
        const auto k_ij = edge->k_ij(frame_index);
        const auto k_ji = edge->k_ji(frame_index);
        spdlog::debug("   Nbr {}. k_ij = {}, k_ji = {}", nbr_surfel->id(), k_ij, k_ji);

        // Compute the angle between this surfel in this frame and the neighbour in this frame.
        auto r4_oi_ni_kij = vector_by_rotating_around_n(tangent, normal, k_ij);
        auto r4_oj_nj_kji = vector_by_rotating_around_n(nbr_tangent, nbr_normal, k_ji);
        float theta = degrees_angle_between_vectors(r4_oi_ni_kij, r4_oj_nj_kji);

        // theta = -180 to 180 but we want -45 to 45
        spdlog::debug("   Orig theta {}", theta);
        theta = fmod(theta, 90.0f);
        if( theta > 45 ) theta = 90 - theta;
        spdlog::debug("   Mod theta {}", theta);
        const auto smoothness = (theta * theta);
        spdlog::debug("   % smooth {}", smoothness);

        frame_smoothness += smoothness;
        spdlog::debug("   Frame_smoothness {}", frame_smoothness);
    }

    num_neighbours = neighbours_in_frame.size();
    return frame_smoothness;
}

bool
RoSyOptimiser::compare_worst_first(const SurfelGraphNodePtr &l, const SurfelGraphNodePtr &r) const {
    // Return true if l has higher smoothness : ie is less smooth
    return l->data()->rosy_smoothness() > r->data()->rosy_smoothness();
}

void
RoSyOptimiser::optimise_node(const SurfelGraphNodePtr &this_node) {
    using namespace Eigen;

    static unsigned int iterations = 0;

    const auto &this_surfel = this_node->data();
    const auto &old_tangent = this_surfel->tangent();
    Vector3f new_tangent{old_tangent};

    // For each frame that this surfel appears in.
    for (const auto frame_index : this_surfel->frames()) {
        Eigen::Vector3f vertex, reference_lattice_point, normal, tangent, orth_tangent;
        this_surfel->get_all_data_for_surfel_in_frame(
                frame_index,
                vertex,
                tangent,
                orth_tangent,
                normal,
                reference_lattice_point);


        // For each neighbour j in frame ...
        float w_sum = 0.0f;
        auto neighbours_in_frame = get_node_neighbours_in_frame(m_surfel_graph, this_node, frame_index);
        // Optionally randomise the order
        if (m_randomise_neighour_order) {
            std::shuffle(begin(neighbours_in_frame),
                         end(neighbours_in_frame),
                         m_random_engine);
        }

        for (const auto &neighbour_node : neighbours_in_frame) {
            const auto &nbr_surfel = neighbour_node->data();
            auto edge = m_surfel_graph->edge(this_node, neighbour_node);

            float w_ij = 1.0f;
            float w_ji = 1.0f;
            if (m_weight_for_error) {
                const auto total_smoothness = this_surfel->rosy_smoothness() + nbr_surfel->rosy_smoothness();
                if (total_smoothness != 0) {
                    float delta =
                            (this_surfel->rosy_smoothness() - nbr_surfel->rosy_smoothness()) /
                            (float) total_smoothness;
                    // If delta > 0 we want to weight ij less than ji
                    delta *= (iterations / m_weight_for_error_steps);
                    delta = std::fminf(0.9f, std::fmaxf(-0.9f, delta));
                    w_ij -= delta;
                    w_ji += delta;
                }
            }

            Vector3f neighbour_tan_in_surfel_space, neighbour_norm_in_surfel_space;
            this_surfel->transform_surfel_via_frame(nbr_surfel, frame_index,
                                                    neighbour_norm_in_surfel_space,
                                                    neighbour_tan_in_surfel_space);

            unsigned short k_ij;
            unsigned short k_ji;
            new_tangent = average_rosy_vectors(
                    new_tangent,
                    Vector3f::UnitY(),
                    w_sum,
                    neighbour_tan_in_surfel_space,
                    neighbour_norm_in_surfel_space,
                    w_ji,
                    k_ij,
                    k_ji);
            w_sum += w_ij;

            // Store ks
            edge->set_k_ij(frame_index, k_ij);
            edge->set_k_ji(frame_index, k_ji);
        }
    }

    if (m_weight_for_error) {
        ++iterations;
    }

    if (m_damping_factor > 0) {
        new_tangent = (m_damping_factor * old_tangent) + ((1.0f - m_damping_factor) * new_tangent);
        new_tangent = project_vector_to_plane(new_tangent, Vector3f::UnitY(), true);
    }
    this_node->data()->setTangent(new_tangent);
    auto vec_pair = best_rosy_vector_pair(new_tangent, Vector3f::UnitY(), old_tangent, Vector3f::UnitY());
    this_node->data()->set_rosy_correction(fmod(degrees_angle_between_vectors(vec_pair.first, vec_pair.second), 90.0f));
}
