//
// Created by Dave Durbin on 18/5/20.
//

#include "AbstractOptimiser.h"

#include <utility>
#include <sys/stat.h>
#include <algorithm>
#include <random>

AbstractOptimiser::AbstractOptimiser(Properties properties) : m_state{UNINITIALISED} //
        , m_properties(std::move(properties)) //
        , m_convergence_threshold{1.0} //
        , m_numFrames{0} //
        , m_optimisation_cycles{0} //
        , m_last_smoothness{0.0f} //
{

}

AbstractOptimiser::~AbstractOptimiser() = default;

/**
 * Build the neighbours_by_surfel_frame data structure. Neighbours stay neighbours throughout and so we can compute this once
 * We assume that
 * -- surfels_by_frame is populated for this level
 *
 * But num_frames and num_surfels are both known.
 */
void
AbstractOptimiser::populate_neighbours_by_surfel_frame() {
    using namespace std;

    m_neighbours_by_surfel_frame.clear();

    // For each Surfel
    for (const auto &surfel_ptr_node : m_surfel_graph->nodes()) {
        const auto surfel_ptr = surfel_ptr_node->data();

        // Consider each neighbour
        for (const auto &neighbour : m_surfel_graph->neighbours(surfel_ptr_node)) {

            // And each frame
            for (size_t frame_idx = 0; frame_idx < m_numFrames; ++frame_idx) {
                if (!surfel_ptr->is_in_frame(frame_idx)) {
                    continue;
                }

                if (!neighbour->data()->is_in_frame(frame_idx)) {
                    continue;
                }
                SurfelInFrame sif{surfel_ptr, frame_idx};
                m_neighbours_by_surfel_frame.emplace(sif, neighbour->data());
            }
        }
    }
}

/**
 * Start global smoothing.
 */
void
AbstractOptimiser::begin_optimisation() {
    using namespace spdlog;

    assert(m_state == INITIALISED);

    // Populate map of neighbours for a surfel in a frame
    info("Computing neighbours for each surfel-frame");
    populate_neighbours_by_surfel_frame();

    // Compute initial error values
    info("Initialising error value");
    m_last_smoothness = compute_smoothness_per_surfel();

    m_optimisation_cycles = 0;
    optimisation_began();
    m_state = OPTIMISING;
}

/**
 * Perform post-smoothing tidy up.
 */
void
AbstractOptimiser::optimise_end() {
    assert(m_state == ENDING_OPTIMISATION);

    optimisation_ended();
    // TODO: Consider a final state here that can transition back to INITAILISED or make both READY
    m_state = INITIALISED;
}

/**
 * Perform a single step of optimisation. Return true if converged or halted.
 */
bool
AbstractOptimiser::optimise_do_one_step() {
    assert(m_state != UNINITIALISED);

    if (m_state == INITIALISED) {
        begin_optimisation();
    }

    if (m_state == OPTIMISING) {
        auto nodes_to_optimise = select_nodes_to_optimise();
        for (const auto &node : nodes_to_optimise) {
            optimise_node(node);
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
AbstractOptimiser::select_nodes_to_optimise() const {
    using namespace std;

    assert(m_node_selection_function);
    return m_node_selection_function(*this);
}

/**
 * Check whether the user cancelled optimisation by creating the
 * halt file.
 */
void
AbstractOptimiser::check_cancellation() {
    struct stat buffer{};
    auto rv = stat("halt", &buffer);
    if (rv == 0) {
        m_state = ENDING_OPTIMISATION;
    }
}

float
AbstractOptimiser::compute_surfel_smoothness_for_frame(const std::shared_ptr<Surfel> &surfel_ptr,
                                                       size_t frame_id) const {
    float total_smoothness = 0.0f;

    // Get this surfels stats for the frame
    Eigen::Vector3f this_surfel_position, this_surfel_tangent, this_surfel_normal;
    surfel_ptr->get_position_tangent_normal_for_frame(frame_id //
            , this_surfel_position //
            , this_surfel_tangent //
            , this_surfel_normal
    );

    // For each neighbour in this frame
    const auto &bounds = m_neighbours_by_surfel_frame.equal_range({surfel_ptr, frame_id});
    unsigned int num_neighbours = 0;
    for (auto iter = bounds.first; iter != bounds.second; ++iter) {
        const auto &neighbour_ptr = iter->second;

        // Get the neighbours stats for the frame
        Eigen::Vector3f neighbour_surfel_position, neighbour_surfel_tangent, neighbour_surfel_normal;
        neighbour_ptr->get_position_tangent_normal_for_frame(frame_id //
                , neighbour_surfel_position //
                , neighbour_surfel_tangent //
                , neighbour_surfel_normal
        );

        // Compute the smoothness over this surfel in this frame and the neighbour in this frame.
        total_smoothness += compute_smoothness(
                this_surfel_position,
                this_surfel_tangent,
                this_surfel_normal,
                surfel_ptr->closest_mesh_vertex_offset,

                // Neighbours norm tan pos in surfel frame of reference
                neighbour_surfel_position,
                neighbour_surfel_tangent,
                neighbour_surfel_normal,
                neighbour_ptr->closest_mesh_vertex_offset);

        ++num_neighbours;
    }
    return (num_neighbours == 0)
           ? 0.0f
           : (total_smoothness / (float) num_neighbours);
}

float
AbstractOptimiser::compute_surfel_smoothness(const std::shared_ptr<Surfel> &surfel) const {
    float total_smoothness = 0.0f;

    // For each frame in which this surfel appears
    for (const auto &frame_data : surfel->frame_data) {
        // Compute the smoothness in this frame
        total_smoothness += compute_surfel_smoothness_for_frame(surfel, frame_data.pixel_in_frame.frame);
    }
    // Return mean smoothness per frame
    (std::const_pointer_cast<Surfel>(surfel))->posy_smoothness = total_smoothness / (float) surfel->frames.size();
    return surfel->posy_smoothness;
}


float
AbstractOptimiser::compute_smoothness_per_surfel() const {
    float total_smoothness = 0.0f;
    for (const auto &n : m_surfel_graph->nodes()) {
        total_smoothness += compute_surfel_smoothness(n->data());
    }
    return total_smoothness / (float) m_surfel_graph->num_nodes();
}

/**
 * Check whether optimisation has converged.
 */
void
AbstractOptimiser::check_convergence() {
    using namespace spdlog;

    float current_smoothness = compute_smoothness_per_surfel();
    float improvement = m_last_smoothness - current_smoothness;
    m_last_smoothness = current_smoothness;
    float pct = (100.0f * improvement) / m_last_smoothness;
    info("Mean error per surfel: {} ({}%)", current_smoothness, pct);
    if ((pct >= 0) && (std::abs(pct) < m_convergence_threshold)) {
        m_state = ENDING_OPTIMISATION;
    }
}


unsigned int
AbstractOptimiser::count_number_of_frames(const SurfelGraphPtr &surfel_graph) {
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
AbstractOptimiser::set_data(const SurfelGraphPtr &surfel_graph) {
    m_surfel_graph = surfel_graph;
    m_state = INITIALISED;
    m_numFrames = count_number_of_frames(surfel_graph);
}

/**
 * Select all surfels in a layer and randomize the order
 */
std::vector<SurfelGraphNodePtr>
AbstractOptimiser::ssa_select_all_in_random_order() {
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