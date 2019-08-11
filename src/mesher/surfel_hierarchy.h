//
// Created by Dave Durbin on 2019-08-11.
//

#ifndef ANIMESH_SURFEL_HIERARCHY_H
#define ANIMESH_SURFEL_HIERARCHY_H

#include <vector>
#include "types.h"

class SurfelHierarchy {
public:
    /**
     * @return The number of levels in the hierarchy
     */
    int num_levels() const { return levels.size();}

    /**
     * Smooth the hierarchy.
     */
     void optimise( );

    explicit SurfelHierarchy( float convergence_threshold ) {
        this->convergence_threshold = convergence_threshold;
        is_optimising = false;
        is_starting_new_level = false;
        optimising_converged = false;
        last_smoothing_error = 0.0f;
        smoothing_level = -1;
    };

private:
    std::vector<std::vector<Surfel>> levels;

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
    unsigned int smoothing_level;

    /**
     * If true, smoothing has just begun a new level.
     */
    bool is_starting_new_level;

    /**
     * The last computed error for the given layer of the surfel network.
     */
    float last_smoothing_error;

    /**
     * %age change in error between iterations below which we consider convergence to have occurred
     * Expressed as value [0.0 .. 1.0]
     */
    float convergence_threshold;

    /**
     * Start global smoothing.
     */
    void optimise_begin();

    /**
     * Perform a single step of optimisation.
     */
    void optimise_do_one_step();

    /**
     * Perform post-smoothing tidy up.
     */
    void optimise_end();

    /**
     * Do start of level set up. Mostly computing current residual error in this level.
     */
    void optimise_begin_level();

    /**
     * Tidy up at end of level and propagate values down to next level or else flag smoothing as converged
     * if this was level 0.
     */
    void optmise_end_level( );

        /**
         * Select one random surfel and smooth with neighbours
         */
    void optimise_do_one_surfel();

    /**
     * Measure the change in error. If it's below some threshold, consider this level converged.
     */
    bool check_convergence( );

    /**
     * Tidy up at end of level and propagate values down to next level or else flag smoothing as converged
     * if this was level 0.
     */
    void optimise_end_level( );

    /**
     * Calculate remaining error.
     */
    float compute_error( );

    /**
     * Check whether optimising should stop either because user asked for that to happen or else
     * because convergence has happened.
     */
    bool optimising_should_continue();

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
    compute_new_tangent_for_surfel(std::size_t level_idx, std::size_t surfel_idx);
    /**
     * Populate tangents and normals with all eligible tangents, normals from surfel's neighbours
     * tan/norm is eligible iff the neighbour and surfel share a common frame
     * tan/norm are converted to the orignating surfel's frame of reference.
     */
    void
    get_eligible_normals_and_tangents(std::size_t level_idx,
                                      std::size_t surfel_idx,
                                      std::vector<Eigen::Vector3f> &eligible_normals,
                                      std::vector<Eigen::Vector3f> &eligible_tangents);
};



#endif //ANIMESH_SURFEL_HIERARCHY_H
