//
// Created by Dave Durbin on 18/5/20.
//

#ifndef ANIMESH_POSYOPTIMISER_H
#define ANIMESH_POSYOPTIMISER_H

#include <Properties/Properties.h>
#include <Graph/Graph.h>
#include "AbstractOptimiser.h"
#include "../../../mesher/types.h"


class PoSyOptimiser : public AbstractOptimiser {

public:
    /**
     * Construct a PoSyOptimiser.
     * @param properties Parameters for the optimiser.
     */
    explicit PoSyOptimiser(Properties properties);

    ~PoSyOptimiser() override;

protected:
    void optimisation_began() override;

    void optimisation_ended() override;

    void optimise_node(const SurfelGraphNodePtr &node) override;

    /**
     * Surfel selection model 2: Select top 100 error scores
     */
    std::vector<SurfelGraphNodePtr>
    ssa_select_worst_100();

private:
    float m_rho;

    float
    compute_smoothness(const Eigen::Vector3f &position1,
                       const Eigen::Vector3f &tangent1,
                       const Eigen::Vector3f &normal1,
                       const Eigen::Vector2f &uv1,
                       const Eigen::Vector3f &position2,
                       const Eigen::Vector3f &tangent2,
                       const Eigen::Vector3f &normal2,
                       const Eigen::Vector2f &uv2) const override;

    Eigen::Vector2f compute_distortion(
            const Eigen::Vector3f &surfel_position1,
            const Eigen::Vector3f &o1,
            const Eigen::Vector3f &o_prime1,
            const Eigen::Vector2f &uv1,

            const Eigen::Vector3f &surfel_position2,
            const Eigen::Vector3f &o2,
            const Eigen::Vector3f &o_prime2,
            const Eigen::Vector2f &uv2
    ) const;
    };


#endif //ANIMESH_POSYOPTIMISER_H
