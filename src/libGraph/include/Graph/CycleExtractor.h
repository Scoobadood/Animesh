//
// Created by Dave Durbin (Old) on 30/8/21.
//

#pragma once

#include <vector>
#include <queue>
#include "Graph.h"
#include <spdlog/spdlog.h>
#include <algorithm>

namespace animesh {

template<class N, class E> class CycleExtractor {
public:
  using GraphPtr = std::shared_ptr<Graph < N, E>>;
  using GraphNodePtr = std::shared_ptr<typename Graph<N, E>::GraphNode>;

  /*
   * Extract all cycles from graph starting at node.
   * Returns unique, smallest cycles.
   */
  static void extract_cycles(
      const GraphPtr graph,
      std::set<std::vector<GraphNodePtr>> &cycles) {
    using namespace std;

    queue<vector<GraphNodePtr>> candidate_paths;
    for (const auto &node : graph->nodes()) {
      vector<GraphNodePtr> candidate_path;
      candidate_path.push_back(node);
      candidate_paths.push(candidate_path);
    }
    extract_cycles(graph, candidate_paths, cycles);
  }

  /*
   * Extract all cycles from graph starting at node.
   * Returns unique, smallest cycles.
   */
  static void extract_cycles(
      const GraphPtr graph,
      std::queue<std::vector<GraphNodePtr>> &candidate_paths,
      std::set<std::vector<GraphNodePtr>> &cycles
  ) {
    using namespace std;

    set<pair<GraphNodePtr, GraphNodePtr>> traversed_edges;

    while (!candidate_paths.empty()) {
      // DEBUG
//      queue<vector<GraphNodePtr>> dup_queue{candidate_paths};
//      while (!dup_queue.empty()) {
//        auto v = dup_queue.front();
//        dup_queue.pop();
//        for (const auto x : v) {
//          cout << x->data() << " ";
//        }
//        cout << endl;
//      }
//      cout << endl;
      // DEBUG
      const auto candidate_path = candidate_paths.front();
      candidate_paths.pop();

      if( cycle_exists(candidate_path, cycles) ) {
        continue;
      }

      auto eligible_next_nodes = get_eligible_next_nodes(graph, candidate_path, traversed_edges);

      // Check if any of these can close a cycle
      bool cycle_found = false;
      for (const auto &node : eligible_next_nodes) {
        if (can_close_cycle(graph, candidate_path, node, traversed_edges)) {
          vector<GraphNodePtr> cycle{candidate_path};
          cycle.push_back(node);
          if (!cycle_exists(cycle, cycles)) {
            cycles.emplace(cycle);
            mark_edges_used(cycle, traversed_edges);
            cycle_found = true;
          }
        }
      }
      if (cycle_found)
        continue;

      // Get eligible neighbours.
      sort(begin(eligible_next_nodes),
           end(eligible_next_nodes),
           [](const GraphNodePtr &p1, const GraphNodePtr &p2) -> bool {
             return p1->data() < p2->data();
           });

      // Otherwise recurse with each possible neighbour
      for (const auto &nbr : eligible_next_nodes) {
        vector<GraphNodePtr> next_path{candidate_path};
        next_path.push_back(nbr);
        candidate_paths.push(next_path);
      }
    }
  }

  static bool
  cycle_exists(
      const std::vector<GraphNodePtr> &cycle,
      const std::set<std::vector<GraphNodePtr>> &cycles
  ) {
    using namespace std;

    // Check permutations
    vector<GraphNodePtr> perm_cycle{cycle};
    for( int i=0; i < cycle.size(); ++i ) {
      rotate(begin(perm_cycle), begin(perm_cycle) + 1,  end(perm_cycle));
      if( cycles.count(perm_cycle) != 0) {
        return true;
      }
    }

    reverse(begin(perm_cycle), end(perm_cycle));
    for( int i=0; i < cycle.size(); ++i ) {
      rotate(begin(perm_cycle), begin(perm_cycle) + 1, end(perm_cycle));
      if( cycles.count(perm_cycle) != 0) {
        return true;
      }
    }

    return false;
  }

  /*
   * Eligible nodes are those which are reachable by an egde that has not beebn traversed yet
   * and which are not in the path that's already traversed (unless they are at the front and the path length is
   * greater than two.
   */
  static std::vector<GraphNodePtr>
  get_eligible_next_nodes(const GraphPtr &graph,
                          const std::vector<GraphNodePtr> &path_candidate,
                          std::set<std::pair<GraphNodePtr, GraphNodePtr>> &traversed_edges) {
    using namespace std;


    const auto &node = path_candidate.back();
    auto all_neighbours = graph->neighbours(node, true);
    vector<GraphNodePtr> eligible_neighbours;
    // Exclude any neighbours whose edges have been traversed
    for (const auto &neighbour : all_neighbours) {
      // Make the edge test
      pair<GraphNodePtr, GraphNodePtr> key = pair<GraphNodePtr, GraphNodePtr>{node, neighbour};
      if (traversed_edges.count(key) != 0) {
        continue;
      }

      // Make the path test
      const auto iter = find(begin(path_candidate), end(path_candidate), neighbour);
      if (iter != end(path_candidate) &&
          (iter != begin(path_candidate) || path_candidate.size() == 2)) {
        continue;
      }

      eligible_neighbours.push_back(neighbour);
    }
    return eligible_neighbours;
  }

  /**
   * Candidate path is a cycle if there's an edge from last node to first node in the graph
   * and that edge is not in traversed edges.
   */
  static bool
  can_close_cycle(const GraphPtr &graph,
                  const std::vector<GraphNodePtr> &candidate_path,
                  const GraphNodePtr &node,
                  const std::set<std::pair<GraphNodePtr, GraphNodePtr>> &traversed_edges
  ) {
    // Can't be a cycle if path is less than 2
    if (candidate_path.size() < 2) {
      return false;
    }

    // Can't be a cycle if no edge exists in graph from node to front
    auto to_node = candidate_path.front();
    if (!graph->has_edge(node, to_node) &&
        !graph->has_edge(to_node, node)) {
      return false;
    }

    // Can't be a cycle if that edge is already traversed
    if (traversed_edges.count({node, to_node}) != 0) {
      return false;
    }
    // It is a cycle!
    return true;
  }

  static bool
  edges_are_used(
      const std::vector<GraphNodePtr> &path_candidate,
      std::set<std::pair<GraphNodePtr, GraphNodePtr>> &traversed_edges
  ) {
    using namespace std;

    if (traversed_edges.count(
        pair<GraphNodePtr, GraphNodePtr>{path_candidate.back(), path_candidate.front()}
    ) != 0) {
      return true;
    }
    auto from = begin(path_candidate);
    auto to = from + 1;
    while (to != end(path_candidate)) {
      if (traversed_edges.count({*from, *to}) > 0) {
        return true;
      }
      from++;
      to++;
    }
    return false;
  }

  static void
  mark_edges_used(
      const std::vector<GraphNodePtr> &path_candidate,
      std::set<std::pair<GraphNodePtr, GraphNodePtr>> &traversed_edges
  ) {
    traversed_edges.template emplace(path_candidate.back(), path_candidate.front());
    auto from = path_candidate.begin();
    auto to = from + 1;
    while (to != end(path_candidate)) {
      traversed_edges.emplace(*from, *to);
      from++;
      to++;
    }
  }

  /**
   * The canonical form of a path places the lowest value node first
   * Then the lowest of the two possible successors.
   */
  static std::vector<GraphNodePtr>
  canonicalise_path(const std::vector<GraphNodePtr> &path) {
    using namespace std;

    int first_idx = 0;
    for (int idx = 1; idx < path.size(); idx++) {
      if (path[idx]->data() < path[first_idx]->data()) {
        first_idx = idx;
      }
    }

    vector<GraphNodePtr> canonical_path;
    canonical_path.push_back(path[first_idx]);
    auto prev = path[first_idx > 0 ? first_idx - 1 : path.size() - 1];
    auto next = path[first_idx < path.size() - 1 ? first_idx + 1 : 0];
    int inc = (next->data() < prev->data()) ? 1 : -1;
    int idx = first_idx;
    for (int i = 1; i < path.size(); i++) {
      idx += inc;
      if (idx < 0)
        idx = path.size() - 1;
      else if (idx == path.size())
        idx = 0;
      canonical_path.push_back(path[idx]);
    }
    return canonical_path;
  }
};

}



