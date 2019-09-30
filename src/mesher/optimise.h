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

    Optimiser(float convergence_threshold, size_t num_frames) {
        is_optimising = false;
        last_optimising_error = 0.0;
        optimising_converged = false;
        this->convergence_threshold = convergence_threshold;
        this->num_frames = num_frames;
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
     * Useful cache for error computation. Stores a list of surfels which are neighbours of the gievn surfels in frame
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
    void optimise_do_one_surfel(std::vector<Surfel>& surfels);

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
     * Compute a new tangent for the given surfel S
     *
     * for each neighbouring surfel N of S
     *   find any frame f in which both N and S are visible
     *   obtain MfS the transformation matrix from frame f for S
     *   obtain dirNS from MfS * dirN
     *   perform 4RoSy smoothing operation on dirS and dirNS
     * end
     */
    Eigen::Vector3f
    compute_new_tangent_for_surfel(const std::vector<Surfel> &surfels, std::size_t surfel_idx) const;

    /**
     * Populate tangents and normals with all eligible tangents, normals from surfel's neighbours
     * tan/norm is eligible iff the neighbour and surfel share a common frame
     * tan/norm are converted to the orignating surfel's frame of reference.
     */
    void
    get_eligible_normals_and_tangents(const std::vector<Surfel> &surfels,
                                      std::size_t surfel_idx,
                                      std::vector<Eigen::Vector3f> &eligible_normals,
                                      std::vector<Eigen::Vector3f> &eligible_tangents) const;

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


};
