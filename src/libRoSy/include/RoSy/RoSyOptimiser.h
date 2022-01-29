#pragma once

#include <Optimise/NodeOptimiser.h>
#include <Properties/Properties.h>
#include <Surfel/SurfelGraph.h>

class RoSyOptimiser : public NodeOptimiser {
 public:
  RoSyOptimiser(const Properties &properties, std::default_random_engine &rng);

  virtual ~RoSyOptimiser() = default;

 protected:
  void loaded_graph() override {};
  void smoothing_completed(float smoothness, OptimisationResult result) override {};
  void ended_optimisation() override;

 private:
  bool compare_worst_first(const SurfelGraphNodePtr &l, const SurfelGraphNodePtr &r) const override;

  float compute_smoothness_in_frame(const SurfelGraph::Edge &edge, unsigned int frame_idx) const override;

  const std::string &get_ssa_property_name() const override {
    static const std::string SSA_PROPERTY_NAME = "rosy-surfel-selection-algorithm";
    return SSA_PROPERTY_NAME;
  }

  const std::string &get_ssa_percentage_property_name() const override {
    static const std::string SSA_PERCENTAGE_PROPERTY_NAME = "rosy-ssa-percentage";
    return SSA_PERCENTAGE_PROPERTY_NAME;
  }

  void vote_for_best_ks(
      const std::shared_ptr<Surfel> &this_surfel,
      const std::shared_ptr<Surfel> &that_surfel,
      const Eigen::Vector3f &tangent,
      std::vector<unsigned int> &shared_frames,
      unsigned short &best_k_ij,
      unsigned short &best_k_ji) const;

  void optimise_node(const SurfelGraphNodePtr &this_node) override;
  void store_mean_smoothness(SurfelGraphNodePtr node, float smoothness) const override;
  void adjust_weights_based_on_error(const std::shared_ptr<Surfel> &s1,
                                     const std::shared_ptr<Surfel> &s2,
                                     float &w_ij,
                                     float &w_ji) const;
  void get_weights(const std::shared_ptr<Surfel> &surfel_a,
                   const std::shared_ptr<Surfel> &surfel_b,
                   float &weight_a,
                   float &weight_b) const;

  // Per edge/per frame
  struct FrameStat {
    unsigned short best_kij;
    unsigned short best_delta;
    float best_dp;
  };

  void compute_all_dps(
      const std::shared_ptr<Surfel> &s1,
      const std::shared_ptr<Surfel> &s2,
      unsigned int num_frames,
      std::vector<std::vector<std::vector<float>>> &dot_prod,
      std::vector<FrameStat> &frame_stats
  ) const;

  /*
 * Smooth an individual Surfel across temporal and spatial neighbours.
 */
  void optimise_node_with_one_neighbour(const SurfelGraphNodePtr &this_node, //
                                        const std::shared_ptr<Surfel> &this_surfel,
                                        Eigen::Vector3f &new_tangent //
  );
  void optimise_node_with_all_neighbours(const SurfelGraphNodePtr &this_node, //
                                        const std::shared_ptr<Surfel> &this_surfel,
                                        Eigen::Vector3f &new_tangent //
  );
  float m_damping_factor;
  bool m_weight_for_error;
  bool m_vote_for_best_k;
  int m_weight_for_error_steps;
};