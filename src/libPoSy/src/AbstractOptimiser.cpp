//
// Created by Dave Durbin on 18/5/20.
//

#include "AbstractOptimiser.h"

#include <utility>
#include <sys/stat.h>
#include <algorithm>
#include <random>

AbstractOptimiser::AbstractOptimiser(Properties properties) : m_properties(std::move(properties)),
                                                              m_state{UNINITIALISED},
                                                              m_optimisation_cycles{0},
                                                              m_last_smoothness{0.0f},
                                                              m_convergence_threshold{1.0} {
}

AbstractOptimiser::~AbstractOptimiser() = default;

/**
 * Build the norm_tan_by_surfel_frame data structure for this level of the
 * surfel hierarchy. Note that tans will be updated every optimisation
 * We should calculate this once per level and then update each time we change a tan.
 */
void
AbstractOptimiser::populate_norm_tan_by_surfel_frame() {
    using namespace std;
    using namespace Eigen;

    m_norm_tan_by_surfel_frame.clear();

    /*
       For each surfel
       For each framedata in the surfel
       Generate a surfel-frame pair
       Transform the norm and tan
       Put it in the map.
     */
    for (const auto &n : m_surfel_graph.nodes()) {
        const auto &surfel_ptr = n->data();
        for (const auto &fd : surfel_ptr->frame_data) {
            SurfelInFrame surfel_in_frame{surfel_ptr, fd.pixel_in_frame.frame};
            Vector3f new_tan = fd.transform * surfel_ptr->tangent;
            m_norm_tan_by_surfel_frame.emplace(surfel_in_frame, NormalTangent{fd.normal, new_tan});
        }
    }
}

/**
 * Build a mapping from frame to the Surfels which appear in that frame.
 * This allows us to work with frames individually.
 * surfels_by_frame is a member variable.
 */
void
AbstractOptimiser::populate_frame_to_surfel() {
    using namespace std;

    multimap<size_t, shared_ptr<Surfel>> surfels_by_frame;

    // Push an empty vector of Surfel references for each frame
    m_surfels_by_frame.clear();
    // For each Surfel add it to each frame in which it appears.
    size_t max_frame_index = 0;
    for (const auto &n : m_surfel_graph.nodes()) {
        const auto &surfel = n->data();
        for (const auto &fd : surfel->frame_data) {
            surfels_by_frame.emplace(fd.pixel_in_frame.frame, surfel);
            if (fd.pixel_in_frame.frame > max_frame_index) {
                max_frame_index = fd.pixel_in_frame.frame;
            }
        }
    }

    m_surfels_by_frame.reserve(max_frame_index + 1);
    for (auto i = 0; i < max_frame_index; i++) {
        m_surfels_by_frame.emplace_back();
    }
    for (size_t frame_idx = 0; frame_idx < max_frame_index; ++frame_idx) {
        const auto &range = surfels_by_frame.equal_range(frame_idx);
        for (auto it = range.first; it != range.second; ++it) {
            m_surfels_by_frame.at(it->first).emplace_back(it->second);
        }
    }
}

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

    assert(!m_surfels_by_frame.empty());
    m_neighbours_by_surfel_frame.clear();

    const auto num_frames = m_surfels_by_frame.size();

    // For each Surfel
    for (const auto &surfel_ptr_node : m_surfel_graph.nodes()) {
        const auto surfel_ptr = surfel_ptr_node->data();

        // Consider each neighbour
        for (const auto &neighbour : m_surfel_graph.neighbours(surfel_ptr_node)) {

            // And each frame
            for (size_t frame_idx = 0; frame_idx < num_frames; ++frame_idx) {
                if (!surfel_is_in_frame(surfel_ptr, frame_idx)) {
                    continue;
                }

                if (!surfel_is_in_frame(neighbour->data(), frame_idx)) {
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

    // First we construct a mapping from frame to surfel
    info("Mapping frames to surfels");
    populate_frame_to_surfel();

    // Populate norm and tangent for each surfel in each frame.
    info("Computing normals and tangents for each frame.");
    populate_norm_tan_by_surfel_frame();

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
    return m_node_selection_function();
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

/**
 * Return a vector of pairs of FrameData for each frame that these surfels have in common
 */
std::vector<std::pair<std::reference_wrapper<const FrameData>, std::reference_wrapper<const FrameData>>>
find_common_frames_for_surfels(const std::shared_ptr<Surfel> &surfel1, const std::shared_ptr<Surfel> &surfel2) {
    using namespace std;

    vector<reference_wrapper<const FrameData>> surfel1_frames;
    vector<reference_wrapper<const FrameData>> surfel2_frames;
    for (const auto &f : surfel1->frame_data) {
        surfel1_frames.emplace_back(f);
    }
    for (const auto &f : surfel2->frame_data) {
        surfel2_frames.emplace_back(f);
    }
    sort(surfel1_frames.begin(), surfel1_frames.end(),
         [](const FrameData &f1, const FrameData &f2) { return f1.pixel_in_frame.frame < f2.pixel_in_frame.frame; });
    sort(surfel2_frames.begin(), surfel2_frames.end(),
         [](const FrameData &f1, const FrameData &f2) { return f1.pixel_in_frame.frame < f2.pixel_in_frame.frame; });

    vector<pair<reference_wrapper<const FrameData>, reference_wrapper<const FrameData>>> common_frames;
    auto it1 = surfel1->frame_data.begin();
    auto it2 = surfel2->frame_data.begin();
    while (it1 != surfel1->frame_data.end() && it2 != surfel2->frame_data.end()) {
        if (it1->pixel_in_frame.frame < it2->pixel_in_frame.frame) {
            ++it1;
        } else if (it2->pixel_in_frame.frame < it1->pixel_in_frame.frame) {
            ++it2;
        } else {
            common_frames.emplace_back(ref(*it1), ref(*it2));
            ++it1;
            ++it2;
        }
    }
    return common_frames;
}

FrameData
AbstractOptimiser::frame_data_for_surfel_in_frame(const std::shared_ptr<Surfel> &surfel_ptr,
                                                  unsigned int frame_index) const {
    for (const auto &fd : surfel_ptr->frame_data) {
        if (fd.pixel_in_frame.frame == frame_index) {
            return fd;
        }
    }
    throw std::runtime_error(
            fmt::format("Frame data not found for surfel {} in frame {}", surfel_ptr->id, frame_index));
}

FrameData
AbstractOptimiser::frame_data_for_surfel_in_frame(const SurfelInFrame &sif) const {
    return frame_data_for_surfel_in_frame(sif.surfel_ptr, sif.frame_index);
}

float
AbstractOptimiser::compute_surfel_smoothness_for_frame(const std::shared_ptr<Surfel> &surfel_ptr,
                                                       size_t frame_id) const {
    float total_smoothness = 0.0f;

    // Get all neighbours in frame
    const SurfelInFrame surfel_in_frame{surfel_ptr, frame_id};
    const auto &surfel_orientation_in_frame = m_norm_tan_by_surfel_frame.at(surfel_in_frame);
    const auto &surfel_frame_data = frame_data_for_surfel_in_frame(surfel_in_frame);

    const auto &bounds = m_neighbours_by_surfel_frame.equal_range(surfel_in_frame);
    unsigned int num_neighbours = 0;
    for (auto np = bounds.first; np != bounds.second; ++np) {
        const auto & neighbour_ptr = np->second;
        const SurfelInFrame neighbour_in_frame{neighbour_ptr, frame_id};
        const auto &neighbour_orientation_in_frame = m_norm_tan_by_surfel_frame.at(neighbour_in_frame);
        const auto &neighbour_frame_data = frame_data_for_surfel_in_frame(neighbour_in_frame);

        // Compute the smoothness over this surfel in this frame and the neighbour in this frame.
        total_smoothness += compute_smoothness(
                surfel_frame_data.position,
                surfel_orientation_in_frame.tangent,
                surfel_orientation_in_frame.normal,
                surfel_ptr->closest_mesh_vertex_offset,

                // Neighbours norm tan pos in surfel frame of reference
                neighbour_frame_data.position,
                neighbour_orientation_in_frame.tangent,
                neighbour_orientation_in_frame.normal,
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
    (std::const_pointer_cast<Surfel>(surfel))->posy_smoothness = total_smoothness / surfel->frame_data.size();
    return surfel->posy_smoothness;
}


float
AbstractOptimiser::compute_smoothness_per_surfel() const {
    float total_smoothness = 0.0f;
    for (const auto &n : m_surfel_graph.nodes()) {
        total_smoothness += compute_surfel_smoothness(n->data());
    }
    return total_smoothness / m_surfel_graph.num_nodes();
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

/**
 * Set the optimisation data
 */
void
AbstractOptimiser::set_data(const SurfelGraph &surfel_graph) {
    m_surfel_graph = surfel_graph;
    m_state = INITIALISED;
}

/**
 * Select all surfels in a layer and randomize the order
 */
std::vector<SurfelGraphNodePtr>
AbstractOptimiser::ssa_select_all_in_random_order() {
    using namespace std;

    vector<size_t> indices(m_surfel_graph.num_nodes());
    iota(begin(indices), end(indices), 0);
    shuffle(begin(indices), end(indices),
            default_random_engine(chrono::system_clock::now().time_since_epoch().count()));
    vector<SurfelGraphNodePtr> selected_nodes;
    selected_nodes.reserve(indices.size());
    const auto graph_nodes = m_surfel_graph.nodes();
    for (const auto i : indices) {
        selected_nodes.push_back(graph_nodes.at(i));
    }
    return selected_nodes;
}

/**
 * Return true if the given Surfel is in the specified frame.
 */
bool
AbstractOptimiser::surfel_is_in_frame(const std::shared_ptr<Surfel> &surfel_ptr, size_t frame_index) const {
    using namespace std;

    for (const auto &fd : surfel_ptr->frame_data) {
        if (fd.pixel_in_frame.frame == frame_index) {
            return true;
        }
    }
    return false;
}
