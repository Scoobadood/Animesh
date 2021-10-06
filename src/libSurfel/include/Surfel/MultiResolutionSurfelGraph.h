//
// Created by Dave Durbin (Old) on 4/10/21.
//

#pragma once

#include "SurfelGraph.h"
#include <vector>

class MultiResolutionSurfelGraph {
public:
  /**
   * Construct from a given starting graph.
   */
  explicit MultiResolutionSurfelGraph(const SurfelGraphPtr &surfel_graph);

  /**
   * Generate levels (if not already done).
   */
  void generate_levels(unsigned int num_levels);

  /**
   * Return a reference to the specified level of the graph
   */
   const SurfelGraphPtr& operator[](size_t index) {
     return m_levels[index];
   }

private:
  struct SurfelGraphEdgeComparator;

  std::vector<SurfelGraphPtr> m_levels;

  std::default_random_engine m_random_engine;

  /**
   * Generate the next level for this multi-resolution graph.
   */
  void generate_new_level();

  /**
   * Compute the mean normal for the given node across
   * all frames in which it appears.
   */
  static Eigen::Vector3f
  compute_mean_normal(const SurfelGraphNodePtr &graph_node_ptr);

  /**
   * Compute the scores for each edge.
   */
  static void compute_scores(const SurfelGraphPtr &current_graph,
                      const std::map<std::string, int> &dual_area_by_node,
                      const std::map<std::string, Eigen::Vector3f> &mean_normal_by_node,
                      std::map<SurfelGraph::Edge, float, SurfelGraphEdgeComparator> &scores);
};
