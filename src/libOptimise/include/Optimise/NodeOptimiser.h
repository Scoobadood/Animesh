//
// Created by Dave Durbin (Old) on 8/11/21.
//

#pragma once

#include "AbstractOptimiser.h"
#include <numeric>      // iota

class NodeOptimiser : public AbstractOptimiser {
protected:
  NodeOptimiser(Properties properties, std::default_random_engine &rng);

  void optimise_do_pass() override;

  virtual void optimise_node(const SurfelGraphNodePtr &this_node) = 0;

  virtual bool compare_worst_first(const SurfelGraphNodePtr &l, const SurfelGraphNodePtr &r) const = 0;

  bool m_randomise_neighour_order;

  virtual const std::string &get_ssa_property_name() const = 0;

  virtual const std::string &get_ssa_percentage_property_name() const = 0;

  std::vector<SurfelGraphNodePtr> ssa_select_all_in_random_order();

  std::vector<SurfelGraphNodePtr> ssa_select_worst_100() const;

  std::vector<SurfelGraphNodePtr> ssa_select_worst_percentage() const;

  std::vector<SurfelGraphNodePtr> select_nodes_to_optimise();

  void setup_ssa();

  std::function<std::vector<SurfelGraphNodePtr>(const AbstractOptimiser &)> m_node_selection_function;

  float m_ssa_percentage;
};