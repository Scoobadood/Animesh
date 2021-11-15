//
// Created by Dave Durbin (Old) on 4/5/21.
//

#pragma once

#include <Properties/Properties.h>
#include <Surfel/SurfelGraph.h>
#include "Optimiser.h"

#include <random>

class AbstractOptimiser : public Optimiser {
public:
  void set_data(const SurfelGraphPtr &surfel_graph) override;

  bool optimise_do_one_step() override;


protected:
  AbstractOptimiser(Properties properties, std::default_random_engine& rng);

  void setup_termination_criteria(
      const std::string &termination_criteria_property,
      const std::string &relative_smoothness_property,
      const std::string &absolute_smoothness_property,
      const std::string &max_iterations_property);

  enum OptimisationResult {
    NOT_COMPLETE,
    CONVERGED,
    CANCELLED,
  };

  /* Call back once a graph is loaded to provide an opportunity to play with it before smoothing starts */
  virtual void loaded_graph() {};
  /* Call back when termination criteria are met */
  virtual void smoothing_completed(float smoothness, OptimisationResult result);
  virtual void trace_smoothing(const SurfelGraphPtr &graph) const {};
  virtual void optimise_do_pass() = 0;

  enum OptimisationState {
    UNINITIALISED,
    INITIALISED,
    OPTIMISING,
    ENDING_OPTIMISATION
  };

  OptimisationResult m_result;
  OptimisationState m_state;
  SurfelGraphPtr m_surfel_graph;

  static std::vector<unsigned int>
  get_common_frames(const std::shared_ptr<Surfel> &s1, const std::shared_ptr<Surfel> &s2) ;

private:
  // Optimisation
  void optimise_begin();

  void optimise_end();

  void compute_smoothness(float & mean_node_smoothness, std::vector<float> frame_smoothness);
  virtual float compute_smoothness_in_frame( const SurfelGraph::Edge & edge, unsigned int frame_idx) const = 0;
  virtual void store_mean_smoothness(SurfelGraphNodePtr node, float smoothness) const = 0;

  unsigned short read_termination_criteria(const std::string &termination_criteria);

  // Checking for termination
  static bool user_canceled_optimise();

  static bool check_cancellation(OptimisationResult &result);

  bool maybe_check_convergence(float smoothness, OptimisationResult &result);

  bool maybe_check_iterations(OptimisationResult &result) const;

  bool check_termination_criteria(float &smoothness, OptimisationResult &result);

  void extract_graph_statistics();

  // Termination criteria
  static const unsigned short TC_ABSOLUTE = 1 << 0;
  static const unsigned short TC_RELATIVE = 1 << 1;
  static const unsigned short TC_FIXED = 1 << 2;

  unsigned short        m_termination_criteria;
  float                 m_term_crit_absolute_smoothness;
  float                 m_term_crit_relative_smoothness;
  unsigned int          m_term_crit_max_iterations;
  unsigned int          m_num_iterations;
  unsigned int          m_num_frames;
  std::vector<int>      m_nodes_per_frame;
  std::vector<int>      m_edges_per_frame;
  float                 m_last_smoothness;
  bool                  m_show_progress = true;
};