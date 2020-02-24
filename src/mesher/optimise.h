#pragma once

#include "surfel_compute.h"

#include <map>
#include <vector>

class Optimiser {
public:
    /**
     * Perform orientation field optimisation.
     * Continuously step until done.
     */
    void
    optimise(std::vector<Surfel> &surfels);

    Optimiser(float convergence_threshold, size_t num_frames, size_t surfels_per_step) {
        is_optimising = false;
        last_optimising_error = 0.0;
        optimising_converged = false;
        this->convergence_threshold = convergence_threshold;
        this->num_frames = num_frames;
        if( surfels_per_step == 0 ) throw std::runtime_error("surfels per step must be at least 1");
        this->surfels_per_step = surfels_per_step;
    }

private:
    /**
     * Start optimisation
     */
    void
    optimise_begin(const std::vector<Surfel>& surfels);

    /**
     * Perform post-optimisation tidy up.
     */
    void
    optimise_end();

    /**
     * Check whether optimising should stop either because user asked for that to happen or else
     * because convergence has happened.
     */
    bool
    optimising_should_continue();

    /**
     * Flag indicating that smoothing is in progress
     */
    bool is_optimising;

    /**
     * True is smoothing and it has converged otherwise false.
     */
    bool optimising_converged;

    /**
     * Index of cutrent level being smoothed.
     */
    unsigned int optimising_level;

    /**
     * If true, smoothing has just begun a new level.
     */
    bool is_starting_new_level;

    /**
     * The last computed error for the given layer of the surfel network.
     */
    float last_optimising_error;

    /**
     * %age change in error between iterations below which we consider convergence to have occurred
     * Expressed as value [0.0 .. 1.0]
     */
    float convergence_threshold;

    /**
     * Useful cache for error computation. Recalculated per level.
     * A map from a surfel/frame pair to that surfel's transformed normal and tangent in that frame.
     */
    std::map<SurfelInFrame, std::pair<Eigen::Vector3f, Eigen::Vector3f>> norm_tan_by_surfel_frame;

    /**
     * Useful cache for error computation. Stores a list of surfels which are neighbours of the given surfels in frame
     * Key is surfel, frame
     * Value is a vector of const surfel&
     */
    std::map<SurfelInFrame, std::vector<std::reference_wrapper<const Surfel>>> neighbours_by_surfel_frame;

    /**
     * Map from frame to the surfels in it. Populated once per level.
     */
    std::vector<std::vector<std::reference_wrapper<const Surfel>>> surfels_by_frame;

    /**
     * The number of frames in the sequence.
     */
    size_t num_frames;

    /**
     * NUmber of Surfels to adjust each step of optimisation
     */
     size_t surfels_per_step;

    /**
     * Perform a single step of optimisation.
     */
    void optimise_do_one_step(std::vector<Surfel>& surfels);

    /**
     * Do start of level set up. Mostly computing current residual error in this level.
     */
    void optimise_begin_level();

    /**
     * Select one random surfel and smooth with neighbours
     */
    void optimise_one_surfel_frame(std::vector<Surfel>& surfels);

    /**
     * Measure the change in error. If it's below some threshold, consider this level converged.
     */
    bool check_convergence(std::vector<Surfel>& surfels);

    /**
     * Tidy up at end of level and propagate values down to next level or else flag smoothing as converged
     * if this was level 0.
     */
    void optimise_end_level();

    /**
     * Calculate remaining error.
     */
    float compute_error(const std::vector<Surfel>& surfels);

    /**
     * Calculate error between two normal/tangent pairs.
     */
    static float
    compute_error(const std::pair<Eigen::Vector3f, Eigen::Vector3f> &first,
                  const std::pair<Eigen::Vector3f, Eigen::Vector3f> &second);

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
    populate_norm_tan_by_surfel_frame(const std::vector<Surfel>& surfels);

    /**
     * Build the neighbours_by_surfel_frame data structure for this level of the
     * surfel hierarchy. Neighbours stay neighbours throughout and so we can compute this
     * once during surfel hierarchy extraction.
     */
    void
    populate_neighbours_by_surfel_frame(const std::vector<Surfel>& surfels);

    /**
     * Populate the surfels per fram map.
     */
    void
    populate_frame_to_surfel(const std::vector<Surfel>& surfels);

    /**
     * Compute the intersection of the two provided vectors and place the results into the third.
     */
    static void
    compute_intersection_of(std::vector<size_t> neighbours_of_this_surfel,
                            std::vector<std::reference_wrapper<const Surfel>> surfels_in_this_frame,
                            std::vector<std::reference_wrapper<const Surfel>>& neighbours_in_this_frame);

    /**
     * Perform smoothing for a single surfel in a single frame
     * @param surfel_idx The index of the surfel WITHIN the vector
     * @param frame_idx The index of the frame WITHIN the surfel's frame_data
     */
    void
    smooth_surfel_in_frame(std::vector<Surfel>& surfels, size_t surfel_idx, size_t frame_idx );
};
