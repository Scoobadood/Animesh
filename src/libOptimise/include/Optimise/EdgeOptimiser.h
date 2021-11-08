//
// Created by Dave Durbin (Old) on 8/11/21.
//

#pragma once

#include "AbstractOptimiser.h"
#include <numeric>      // iota

class EdgeOptimiser : public AbstractOptimiser {
protected:
  EdgeOptimiser(Properties properties, std::mt19937 &rng);

  void optimise_do_pass() override;

  virtual void optimise_edge( const SurfelGraph::Edge & edge);
};