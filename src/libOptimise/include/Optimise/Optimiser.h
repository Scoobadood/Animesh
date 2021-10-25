//
// Created by Dave Durbin (Old) on 4/5/21.
//

#pragma once

#include <Properties/Properties.h>
#include <Surfel/SurfelGraph.h>
#include <random>

class Optimiser {
public:
  void set_data(const SurfelGraphPtr &surfel_graph);

  bool optimise_do_one_step();

protected:
  Optimiser(Properties properties, std::mt19937& rng);

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
  virtual const std::string &get_ssa_property_name() const = 0;
  virtual const std::string &get_ssa_percentage_property_name() const = 0;

  std::vector<SurfelGraphNodePtr> get_neighbours_of_node_in_frame(
      const SurfelGraphPtr &graph,
      const SurfelGraphNodePtr &node_ptr,
      unsigned int frame_index,
      bool randomise_order = false) const;

  enum OptimisationState {
    UNINITIALISED,
    INITIALISED,
    OPTIMISING,
    ENDING_OPTIMISATION
  };

  Properties m_properties;
  std::mt19937 & m_random_engine;
  OptimisationResult m_result;
  OptimisationState m_state;
  SurfelGraphPtr m_surfel_graph;
  bool m_randomise_neighour_order;

private:
  // Optimisation
  void optimise_begin();

  void optimise_end();

  virtual void optimise_node(const SurfelGraphNodePtr &node) = 0;

  std::vector<SurfelGraphNodePtr> ssa_select_all_in_random_order();

  std::vector<SurfelGraphNodePtr> ssa_select_worst_100() const;

  std::vector<SurfelGraphNodePtr> ssa_select_worst_percentage() const;

  std::vector<SurfelGraphNodePtr> select_nodes_to_optimise();

  void setup_ssa();

  std::function<std::vector<SurfelGraphNodePtr>(const Optimiser &)> m_node_selection_function;

  virtual bool compare_worst_first(const SurfelGraphNodePtr &l, const SurfelGraphNodePtr &r) const = 0;

  float compute_mean_smoothness() const;
  float compute_mean_node_smoothness(const SurfelGraphNodePtr &node_ptr) const;
  virtual float compute_node_smoothness_for_frame(
      const SurfelGraphNodePtr &node_ptr,
      size_t frame_index,
      unsigned int &num_neighbours) const = 0;
  virtual void store_mean_smoothness(SurfelGraphNodePtr node, float smoothness) const = 0;

  unsigned short read_termination_criteria(const std::string &termination_criteria);

  // Checking for termination
  static bool user_canceled_optimise();

  static bool check_cancellation(OptimisationResult &result);

  bool maybe_check_convergence(float &latest_smoothness, OptimisationResult &result) const;

  bool maybe_check_iterations(OptimisationResult &result) const;

  bool check_termination_criteria(float &smoothness, OptimisationResult &result) const;

  // Termination criteria
  static const unsigned short TC_ABSOLUTE = 1 << 0;
  static const unsigned short TC_RELATIVE = 1 << 1;
  static const unsigned short TC_FIXED = 1 << 2;

  unsigned short        m_termination_criteria;
  float                 m_term_crit_absolute_smoothness;
  float                 m_term_crit_relative_smoothness;
  unsigned int          m_term_crit_max_iterations;
  float                 m_ssa_percentage;
  unsigned int          m_num_iterations;
  unsigned int          m_num_frames;
  float                 m_last_smoothness;
};