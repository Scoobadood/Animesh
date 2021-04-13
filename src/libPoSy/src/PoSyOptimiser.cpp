//
// Created by Dave Durbin on 18/5/20.
//

#include "PoSyOptimiser.h"

#include <utility>
#include <sys/stat.h>
#include <algorithm>
#include <random>
#include <Eigen/Geometry>
#include <RoSy/RoSy.h>
#include <PoSy.h>
#include <Optimise/SurfelSelectionAlgorithm.h>

PoSyOptimiser::PoSyOptimiser(Properties properties)
        : m_properties(std::move(properties)) //
        , m_convergence_threshold{1.0} //
        , m_numFrames{0} //
        , m_optimisation_cycles{0} //
        , m_last_smoothness{0.0f} //
        , m_state{UNINITIALISED} //
{
    m_rho = m_properties.getFloatProperty("rho");
    m_convergence_threshold = m_properties.getFloatProperty("convergence-threshold");
    m_node_selection_function = extractSsa(m_properties);
}

PoSyOptimiser::~PoSyOptimiser() = default;

/**
 * Start global smoothing.
 */
void
PoSyOptimiser::begin_optimisation() {
    assert(m_state == INITIALISED);

    using namespace spdlog;
    info("Initialising error value");

    m_last_smoothness = compute_total_smoothness();
    m_optimisation_cycles = 0;
    m_state = OPTIMISING;
}

/**
 * Perform post-smoothing tidy up.
 */
void
PoSyOptimiser::optimise_end() {
    assert(m_state == ENDING_OPTIMISATION);
    // TODO: Consider a final state here that can transition back to INITAILISED or make both READY
    m_state = INITIALISED;
}

/**
 * Perform a single step of optimisation. Return true if converged or halted.
 */
bool
PoSyOptimiser::optimise_do_one_step() {
    assert(m_state != UNINITIALISED);

    if (m_state == INITIALISED) {
        begin_optimisation();
    }

    if (m_state == OPTIMISING) {
        auto nodes_to_optimise = select_nodes_to_optimise();
        for (const auto &node : nodes_to_optimise) {
            optimise_node(node);
        }

        std::cout << "Round completed. New offsets are: " << std::endl;
        for (const auto &n : m_surfel_graph->nodes()) {
            const auto &v = n->data()->closest_mesh_vertex_offset;
            std::cout << "  " << v.x() << "  " << v.y() << "  " << std::endl;
        }

        ++m_optimisation_cycles;
        check_cancellation();
        check_convergence();
    }

    if (m_state == ENDING_OPTIMISATION) {
        optimise_end();
        return true;
    }
    return false;
}

std::vector<SurfelGraphNodePtr>
PoSyOptimiser::select_nodes_to_optimise() const {
    using namespace std;

    assert(m_node_selection_function);
    return m_node_selection_function(*this);
}

/**
 * Check whether the user cancelled optimisation by creating the
 * halt file.
 */
void
PoSyOptimiser::check_cancellation() {
    struct stat buffer{};
    auto rv = stat("halt", &buffer);
    if (rv == 0) {
        m_state = ENDING_OPTIMISATION;
    }
}

float
PoSyOptimiser::compute_node_smoothness_for_frame(const SurfelGraphNodePtr& node_ptr ,
                                                 size_t frame_index) const {
    float frame_smoothness = 0.0f;

    Eigen::Vector3f this_position, this_tangent, this_normal;
    node_ptr->data()->get_position_tangent_normal_for_frame(frame_index, this_position, this_tangent,
                                                           this_normal);
    // For each neighbour...
    for (const auto &neighbour_node : m_surfel_graph->neighbours(node_ptr)) {

        // If the neighbour's not in the frame, skip
        if (!neighbour_node->data()->is_in_frame(frame_index)) {
            continue;
        }

        Eigen::Vector3f that_position, that_tangent, that_normal;
        neighbour_node->data()->get_position_tangent_normal_for_frame(frame_index, that_position, that_tangent,
                                                                      that_normal);

        auto best_pair = best_rosy_vector_pair(
                this_tangent, this_normal,
                that_tangent, that_normal);

        // Compute the smoothness over this surfel in this frame and the neighbour in this frame.
        frame_smoothness += compute_smoothness(
                this_position,
                best_pair.first,
                this_normal.cross(best_pair.first),
                node_ptr->data()->closest_mesh_vertex_offset,

                that_position,
                best_pair.second,
                that_normal.cross(best_pair.second),
                neighbour_node->data()->closest_mesh_vertex_offset
        );
    }
    return frame_smoothness;
}

float
PoSyOptimiser::compute_node_smoothness(const SurfelGraphNodePtr& node_ptr) const {
    float node_smoothness = 0.0f;

    // For each frame in which this surfel appears
    for (const auto &frame : node_ptr->data()->frames) {
        // Compute the smoothness in this frame
        node_smoothness += compute_node_smoothness_for_frame(node_ptr, frame);
    }
    // Return mean smoothness per frame
    node_ptr->data()->posy_smoothness = node_smoothness / (float)node_ptr->data()->frames.size();
    return node_smoothness;
}

float
PoSyOptimiser::compute_total_smoothness() const {
    float total_smoothness = 0.0f;
    for (const auto &n : m_surfel_graph->nodes()) {
        total_smoothness += compute_node_smoothness(n);
    }
    return total_smoothness;
}

/**
 * Check whether optimisation has converged.
 */
void
PoSyOptimiser::check_convergence() {
    using namespace spdlog;

    float current_smoothness = compute_total_smoothness();
    float improvement = m_last_smoothness - current_smoothness;
    m_last_smoothness = current_smoothness;
    float pct = (100.0f * improvement) / m_last_smoothness;
    info("Mean error per surfel: {} ({}%)", current_smoothness, pct);
    if ((current_smoothness == 0)
        || ((pct >= 0) && (std::abs(pct) < m_convergence_threshold))) {
        m_state = ENDING_OPTIMISATION;
    }
}

unsigned int
PoSyOptimiser::count_number_of_frames(const SurfelGraphPtr &surfel_graph) {
    // Compute the number of frames
    unsigned int max_frame_id = 0;
    for (const auto &n : surfel_graph->nodes()) {
        for (const auto &fd : n->data()->frame_data) {
            if (fd.pixel_in_frame.frame > max_frame_id) {
                max_frame_id = fd.pixel_in_frame.frame;
            }
        }
    }
    return max_frame_id + 1;
}

void
PoSyOptimiser::set_data(const SurfelGraphPtr &surfel_graph) {
    m_surfel_graph = surfel_graph;
    m_state = INITIALISED;
    m_numFrames = count_number_of_frames(surfel_graph);
}

std::function<std::vector<SurfelGraphNodePtr>(const PoSyOptimiser &)>
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
 * Select all surfels in a layer and randomize the order
 */
std::vector<SurfelGraphNodePtr>
PoSyOptimiser::ssa_select_all_in_random_order() {
    using namespace std;

    vector<size_t> indices(m_surfel_graph->num_nodes());
    iota(begin(indices), end(indices), 0);
    shuffle(begin(indices), end(indices),
            default_random_engine(chrono::system_clock::now().time_since_epoch().count()));
    vector<SurfelGraphNodePtr> selected_nodes;
    selected_nodes.reserve(indices.size());
    const auto graph_nodes = m_surfel_graph->nodes();
    for (const auto i : indices) {
        selected_nodes.push_back(graph_nodes.at(i));
    }
    return selected_nodes;
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
    auto new_mesh_vertex_offset = this_surfel_ptr->closest_mesh_vertex_offset;

    float weight = 0.0f;
    Vector2f nudge = Vector2f::Zero();

    // For each frame this surfel is in...
    for (const auto frame_index : this_surfel_ptr->frames) {

        Eigen::Vector3f this_position, this_tangent, this_normal;
        this_surfel_ptr->get_position_tangent_normal_for_frame(frame_index, this_position, this_tangent,
                                                               this_normal);
        // For each neighbour...
        for (const auto &neighbour_node : m_surfel_graph->neighbours(node)) {

            // If the neighbour's not in the frame, skip
            if (!neighbour_node->data()->is_in_frame(frame_index)) {
                continue;
            }

            Eigen::Vector3f that_position, that_tangent, that_normal;
            neighbour_node->data()->get_position_tangent_normal_for_frame(frame_index, that_position, that_tangent,
                                                                          that_normal);

            // TODO(dave.d): We don't want to compute this every time. It should have been stabilised after computing
            // the orientation field and we should store it in the graph on edges.
            auto best_pair = best_rosy_vector_pair(
                    this_tangent, this_normal,
                    that_tangent, that_normal);

            const auto correction = compute_source_uv_correction(
                    this_position,
                    best_pair.first,
                    this_normal.cross(best_pair.first),
                    new_mesh_vertex_offset,

                    that_position,
                    best_pair.second,
                    that_normal.cross(best_pair.second),
                    neighbour_node->data()->closest_mesh_vertex_offset,

                    m_rho
            );

            nudge = ((nudge * weight) + correction) / (weight + 1.0f);
            weight = weight + 1.0f;
            new_mesh_vertex_offset = this_surfel_ptr->closest_mesh_vertex_offset + nudge;
            new_mesh_vertex_offset[0] = remainderf(new_mesh_vertex_offset[0] , m_rho);
            new_mesh_vertex_offset[1] = remainderf(new_mesh_vertex_offset[1] , m_rho);
        } // End of neighbours
    } // End of frames
    this_surfel_ptr->closest_mesh_vertex_offset = new_mesh_vertex_offset;
}

/**
 * Compute the smoothness of the u,v field between one surfel and another.
 * this is calculated as the length of the correction vector required to
 * bring the two views of the lattice into alignment.
 */
float
PoSyOptimiser::compute_smoothness(
        const Eigen::Vector3f &position1,
        const Eigen::Vector3f &tangent_u1,
        const Eigen::Vector3f &tangent_v1,
        const Eigen::Vector2f &uv1,
        const Eigen::Vector3f &position2,
        const Eigen::Vector3f &tangent_u2,
        const Eigen::Vector3f &tangent_v2,
        const Eigen::Vector2f &uv2) const {
    using namespace Eigen;

    auto correction = compute_source_uv_correction(
            position1, tangent_u1, tangent_v1, uv1,
            position2, tangent_u2, tangent_v2, uv2,
            m_rho);
    return (correction[0] * correction[0] + correction[1] * correction[1]);
}