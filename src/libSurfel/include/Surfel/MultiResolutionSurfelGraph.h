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
  MultiResolutionSurfelGraph(const SurfelGraphPtr &surfel_graph,
                             std::mt19937 &rng);

  /**
   * Generate levels (if not already done).
   */
  void generate_levels(unsigned int num_levels);

  /**
   * Up-propagate data from one level to the next.
   */
  void propagate(unsigned int from_level);

  /**
   * Return a reference to the specified level of the graph
   */
  SurfelGraphPtr &operator[](size_t index) {
    return m_levels[index];
  }
  std::shared_ptr<Surfel>
  surfel_merge_function(const std::shared_ptr<Surfel> &n1,
                        float w1,
                        const std::shared_ptr<Surfel> &n2,
                        float w2);

private:
  struct SurfelGraphEdgeComparator;

  std::vector<SurfelGraphPtr> m_levels;

  std::vector<std::map<SurfelGraphNodePtr, std::pair<SurfelGraphNodePtr, SurfelGraphNodePtr>>> m_up_mapping;

  std::mt19937 &m_random_engine;

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
