#pragma once

#include "Surfel.h"
#include "SurfelGraph.h"
#include <Graph/Graph.h>
#include <vector>
#include <string>
#include <memory>

const unsigned int FLAG_SMOOTHNESS = (1 << 1);
const unsigned int FLAG_EDGES = (1 << 0);

/**
 * Save surfel data as binary file to disk
 */
void
save_surfel_graph_to_file(const std::string& file_name,
                          const SurfelGraphPtr& surfel_graph,
                          bool save_smoothness = false,
                          bool save_edges = false
                          );

/**
 * Load surfel data from binary file
 */
SurfelGraphPtr
load_surfel_graph_from_file(const std::string &file_name, std::default_random_engine& rng);

/**
 * Load surfel data from binary file
 */
SurfelGraphPtr
load_surfel_graph_from_file(const std::string &file_name, unsigned short& flags, std::default_random_engine& rng);

