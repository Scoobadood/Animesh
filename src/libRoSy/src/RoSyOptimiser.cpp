#include "RoSy/RoSyOptimiser.h"

#include <RoSy/RoSy.h>
#include <Properties/Properties.h>

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
        const SurfelGraphNodePtr &node_ptr,
        size_t frame_index,
        unsigned int &num_neighbours) const {

    float frame_smoothness = 0.0f;


    Eigen::Vector3f vertex, normal, tangent;
    node_ptr->data()->get_vertex_tangent_normal_for_frame(frame_index, vertex, tangent, normal);

    const auto neighbours_in_frame = get_node_neighbours_in_frame(node_ptr, frame_index);

    // For each neighbour in frame...
    for (const auto &neighbour_node : neighbours_in_frame) {
        Eigen::Vector3f nbr_vertex, nbr_normal, nbr_tangent;
        neighbour_node->data()->get_vertex_tangent_normal_for_frame(frame_index, nbr_vertex, nbr_tangent, nbr_normal);

        // Compute the error between this surfel in this frame and the neighbour in this frame.
        auto vec_pair = best_rosy_vector_pair(tangent, normal, nbr_tangent, normal);
        float theta = degrees_angle_between_vectors(vec_pair.first, vec_pair.second);
        frame_smoothness += (theta * theta) / (45.0*45.0);
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
RoSyOptimiser::optimise_node(const SurfelGraphNodePtr &node) {
    using namespace Eigen;

    static unsigned int iterations = 0;

    const auto &this_surfel_ptr = node->data();
    const auto &old_tangent = this_surfel_ptr->tangent();
    Vector3f new_tangent{old_tangent};

    // For each frame that this surfel appears in.
    for (const auto frame_index : this_surfel_ptr->frames()) {
        Eigen::Vector3f vertex, reference_lattice_point, normal, tangent, orth_tangent;
        this_surfel_ptr->get_all_data_for_surfel_in_frame(
                frame_index,
                vertex,
                tangent,
                orth_tangent,
                normal,
                reference_lattice_point);


        // For each neighbour j in frame ...
        float w_sum = 0.0f;
        auto neighbours_in_frame = get_node_neighbours_in_frame(node, frame_index);
        // Optionally randomise the order
        if(m_randomise_neighour_order) {
            std::shuffle(begin(neighbours_in_frame),
                         end(neighbours_in_frame),
                         m_random_engine);
        }

        for (const auto &neighbour_node : neighbours_in_frame) {
            const auto &that_surfel_ptr = neighbour_node->data();
            auto edge = m_surfel_graph->edge(node, neighbour_node);

            float w_ij = 1.0f;
            float w_ji = 1.0f;
            if( m_weight_for_error) {
                const auto total_smoothness = this_surfel_ptr->rosy_smoothness() + that_surfel_ptr->rosy_smoothness();
                if (total_smoothness != 0) {
                    float delta =
                            (this_surfel_ptr->rosy_smoothness() - that_surfel_ptr->rosy_smoothness()) / (float)total_smoothness;
                    // If delta > 0 we want to weight ij less than ji
                    delta *= (iterations / m_weight_for_error_steps);
                    delta = std::fminf(0.9f, std::fmaxf(-0.9f, delta));
                    w_ij -= delta;
                    w_ji += delta;
                }
            }

            Vector3f neighbour_tan_in_surfel_space, neighbour_norm_in_surfel_space;
            this_surfel_ptr->transform_surfel_via_frame(that_surfel_ptr, frame_index,
                                                        neighbour_norm_in_surfel_space,
                                                        neighbour_tan_in_surfel_space);

            unsigned short source_k;
            unsigned short target_k;
            new_tangent = average_rosy_vectors(
                    new_tangent,
                    Vector3f::UnitY(),
                    w_sum,
                    neighbour_tan_in_surfel_space,
                    neighbour_norm_in_surfel_space,
                    w_ji,
                    target_k,
                    source_k);
            w_sum += w_ij;

            // Store ks
            edge->set_k_ij(frame_index, target_k);
            edge->set_k_ji(frame_index, source_k);
        }
    }

    if( m_weight_for_error) {
        ++iterations;
    }

    if (m_damping_factor > 0) {
        new_tangent = (m_damping_factor * old_tangent) + ((1.0f - m_damping_factor) * new_tangent);
        new_tangent = project_vector_to_plane(new_tangent, Vector3f::UnitY(), true);
    }
    node->data()->setTangent(new_tangent);
    auto vec_pair = best_rosy_vector_pair(new_tangent, Vector3f::UnitY(), old_tangent, Vector3f::UnitY());
    node->data()->set_rosy_correction(fmod(degrees_angle_between_vectors(vec_pair.first, vec_pair.second), 90.0f));
}
