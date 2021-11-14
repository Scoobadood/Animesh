//
// Created by Dave Durbin (Old) on 25/10/21.
//

#pragma once

#include <Properties/Properties.h>
#include <Surfel/SurfelGraph.h>
#include <random>

class Optimiser {
public:
  virtual void set_data(const SurfelGraphPtr &surfel_graph) = 0;

  virtual bool optimise_do_one_step() = 0;

protected:
  Optimiser(Properties properties, std::default_random_engine& rng);

  Properties m_properties;
  std::default_random_engine & m_random_engine;
  SurfelGraphPtr m_surfel_graph;
};