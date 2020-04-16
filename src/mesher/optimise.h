#pragma once

#include "surfel_compute.h"

#include <map>
#include <vector>

class Optimiser {
public:
    /**
     * Perform a single step of optimisation. Return true if converged or halted.
     */
    bool optimise_do_one_step();

    Optimiser(Properties properties);

    std::pair<int, int> get_dimensions() const {
        return std::make_pair(
                m_depth_map_hierarchy.at(m_current_level_index).at(0).width(),
                m_depth_map_hierarchy.at(m_current_level_index).at(0).height()
        );
    };

    /**
     * Set the depth maps and cameras to be used by this optimiser
     * @param depth_maps
     * @param cameras
     */
    void
    set_data(const std::vector<DepthMap> &depth_maps, const std::vector<Camera> &cameras);

    /**
     * Return a const reference to the surfels being transformed so externals can play with it
     */
    const std::vector<Surfel> &get_surfel_data() { return m_current_level_surfels; }


private:
    /** Cameras. One per frame */
    std::vector<Camera> m_cameras;

    /** Surfels in the current level */
    std::vector<Surfel> m_current_level_surfels;

    /** Surfels in the previous level of smoothing if there's more than one. */
    std::vector<Surfel> m_previous_level_surfels;

    /** Properties to use for the optimiser */
    const Properties m_properties;

    /** Number of levels in hierarchy to create */
    size_t m_num_levels = 0;

    /** Index of the level currently being optimised */
    unsigned int m_current_level_index = 0;

    /** Depth maps by level and then frame */
    std::vector<std::vector<DepthMap>> m_depth_map_hierarchy;

    /** Number of cycles of optimisation total */
    unsigned int m_optimisation_cycles;

    /**
     * The last computed error for the given layer of the surfel network.
     */
    float m_last_optimising_error;

    /**
     * %age change in error between iterations below which we consider convergence to have occurred
     * Expressed as value [0.0 .. 1.0]
     */
    float m_convergence_threshold;

    /**
     * Useful cache for error computation. Recalculated per level.
     * A map from a surfel/frame pair to that surfel's transformed normal and tangent in that frame.
     */
    std::map<SurfelInFrame, NormalTangent> m_norm_tan_by_surfel_frame;

    /**
     * Useful cache for error computation. Stores a list of surfels which are neighbours of the given surfels in frame
     * Key is surfel, frame
     * Value is a vector of const surfel&
     */
    std::multimap<SurfelInFrame, std::string> m_neighbours_by_surfel_frame;

    /**
     * Map from frame to the surfels in it. Populated once per level.
     */
    std::vector<std::vector<std::string>> m_surfels_by_frame;

    std::map<SurfelInFrame, bool> m_surfel_frame_map;

    /**
     * The number of frames in the sequence.
     */
    size_t m_num_frames;

    /**
     * Number of Surfels to adjust each step of optimisation
     */
    size_t m_surfels_per_step;

    enum OptimisationState {
        UNINITIALISED,
        INITIALISED,
        STARTING_LEVEL,
        OPTIMISING,
        ENDING_LEVEL,
        ENDING_OPTIMISATION
    } m_state;

    enum OptimisationResult {
        NOT_COMPLETE,
        CONVERGED,
        CANCELLED,
    } m_result;

    /**
     * Start optimisation
     */
    void
    optimise_begin();

    /**
     * Perform post-optimisation tidy up.
     */
    void
    optimise_end();

    /**
     * Do start of level set up. Mostly computing current residual error in this level.
     */
    void optimise_begin_level();

    /**
     * Select one random surfel and smooth with neighbours
     */
    void optimise_one_surfel_frame();

    /**
     * Optimise a single Surfel
     */
    void optimise_surfel(size_t surfel_idx);

    /**
     * Measure the change in error. If it's below some threshold, consider this level converged.
     */
    void check_convergence();

    /**
     * Check whether the user has cancelled this optimisation
     */
    void check_cancellation();

    /**
     * Tidy up at end of level and propagate values down to next level or else flag smoothing as converged
     * if this was level 0.
     */
    void optimise_end_level();

    std::vector<NormalTangent>
    get_eligible_normals_and_tangents(std::size_t surfel_idx) const;

    /**
     * Calculate error between two normal/tangent pairs.
     */
    static float
    compute_error(const NormalTangent &first,
                  const NormalTangent &second);

    float
    compute_surfel_error_for_frame(const std::string &surfel_id, size_t frame_id);

    float
    compute_surfel_error(Surfel &surfel);

    float
    compute_mean_error_per_surfel();

    /**
     * Check whether optimising should stop either because user asked for that to happen or else
     * because convergence has happened.
     */
    static bool user_canceled_optimise();

    /**
     * Select a random integer in the range [0, max_index)
     */
    static std::size_t random_index(unsigned int max_index);

    /**
     * Build the norm_tan_by_surfel_frame data structure for this level of the
     * surfel hierarchy.
     */
    void
    populate_norm_tan_by_surfel_frame();

    /**
     * Build the neighbours_by_surfel_frame data structure for this level of the
     * surfel hierarchy. Neighbours stay neighbours throughout and so we can compute this
     * once during surfel hierarchy extraction.
     */
    void
    populate_neighbours_by_surfel_frame();

    /**
     * Populate the surfels per fram map.
     */
    void
    populate_frame_to_surfel();

    /**
     * Generate surfels for the current optimisation level using
     * correspondences.
     */
    void
    generate_surfels_for_current_level();

    /**
     * Save presmoothed surfels to file if option is set.
     */
    void
    maybe_save_presmooth_surfels_to_file(const Properties &properties);

    /**
     * Save post-smoothed surfels to file if option is set.
     */
    void
    maybe_save_smoothed_surfels_to_file(const Properties &properties);

    /**
     * Compute the intersection of the two provided vectors and place the results into the third.
     */
    static std::vector<std::string>
    compute_intersection_of(std::vector<std::string> neighbours_of_this_surfel,
                            std::vector<std::string> surfels_in_this_frame);

    std::vector<size_t>
    select_surfels_to_optimise();

    bool
    surfel_is_in_frame(const std::string &surfel_id, size_t index);
};