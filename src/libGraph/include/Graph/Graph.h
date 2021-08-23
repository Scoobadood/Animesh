#pragma once

#include <map>
#include <vector>
#include <set>
#include <iostream>
#include <list>
#include <tuple>
#include <string>
#include <sstream>
#include <unordered_set>
#include <spdlog/spdlog.h>
#include <fmt/format.h>

#include "Path.h"

namespace animesh {

/**
* A Graph representation that can handle hierarchical graphs.
* The Graph is constructed over a set of Nodes and Edges
*/
template<class NodeData, class EdgeData> class Graph {
public:
  /**
   * A node in the graph.
   * Nodes are containers for the data they represent
   */
  class GraphNode {
  public:
    /**
     * Construct a GraphNode from data
     */
    explicit GraphNode(const NodeData &data) : m_data{data} {}

    /**
     * Return the data from the node
     */
    inline const NodeData &data() const { return m_data; }

    /**
     * Set the data in the node
     */
    inline void set_data(const NodeData &data) { m_data = data; }

  private:
    /** The data held in this node */
    NodeData m_data;
  };

  using GraphNodePtr = std::shared_ptr<GraphNode>;

  /* ********************************************************************************
  **                                                                            **
  ** Edge                                                                       **
  **                                                                            **
  ********************************************************************************/

  /**
   * An edge in the graph
   */
  class Edge {
  public:
    Edge(const GraphNodePtr from_node, const GraphNodePtr to_node, const std::shared_ptr<EdgeData> edge_data)
        : m_from_node{from_node}, m_to_node{to_node}, m_edge_data{edge_data} {};

    GraphNodePtr from() const { return m_from_node; }

    GraphNodePtr to() const { return m_to_node; }

    std::shared_ptr<EdgeData> data() const { return m_edge_data; }

  private:
    GraphNodePtr m_from_node;
    GraphNodePtr m_to_node;
    std::shared_ptr<EdgeData> m_edge_data;
  };

  /*  ********************************************************************************
      **                                                                            **
      ** Graph public methods                                                       **
      **                                                                            **
      ********************************************************************************/

  /**
   * Make a graph.
   */
  explicit Graph(bool is_directed = false) {
    m_is_directed = is_directed;
  }

  /**
   * Make a node for the given data.
   * @param node_data
   * @return
   */
  static GraphNodePtr make_node(const NodeData &node_data) {
    return std::make_shared<GraphNode>(node_data);
  }

  /**
   * Add a node for the given data. Convenience method.
   * @param data The data to be added.
   */
  GraphNodePtr add_node(const NodeData &data) {
    auto n = make_node(data);
    add_node(n);
    return n;
  }

  /**
   * Add a node for the given data.
   * @param data The data to be added.
   */
  void add_node(const GraphNodePtr &node) {
    check_no_node(node);
    m_nodes.emplace(node);
  }

  /**
   * Remove the given node (and any incident edges)
   */
  void remove_node(const GraphNodePtr &node) {
    check_has_node(node);

    // Find outbound edges from the given node.
    using namespace std;
    auto outbound_edge_key_range = m_nodes_accessible_from.equal_range(node);
    for (auto &map_iter = outbound_edge_key_range.first; map_iter != outbound_edge_key_range.second; ++map_iter) {
      m_edges.erase({node, map_iter->second});
      // Tidy up the reverse adjacency
      auto reverse_key_range = m_nodes_linking_to.equal_range(map_iter->second);
      for (auto &reverse_map_iter = reverse_key_range.first; reverse_map_iter != reverse_key_range.second;
           ++reverse_map_iter) {
        if (reverse_map_iter->second == node) {
          m_nodes_linking_to.erase(reverse_map_iter);
          break;
        }
      }
    }
    m_nodes_accessible_from.erase(outbound_edge_key_range.first, outbound_edge_key_range.second);

    // Now find inbound edges
    auto inbound_edge_key_range = m_nodes_linking_to.equal_range(node);
    for (auto &reverse_map_iter = inbound_edge_key_range.first; reverse_map_iter != inbound_edge_key_range.second;
         ++reverse_map_iter) {
      // Tidy up the forward adjacency
      auto reverse_key_range = m_nodes_accessible_from.equal_range(reverse_map_iter->second);
      for (auto &map_iter = reverse_key_range.first; map_iter != reverse_key_range.second; ++map_iter) {
        if (map_iter->second == node) {
          m_nodes_accessible_from.erase(map_iter);
          m_edges.erase({reverse_map_iter->second, node});
          break;
        }
      }
    }
    m_nodes_linking_to.erase(inbound_edge_key_range.first, inbound_edge_key_range.second);

    m_nodes.erase(node);
  }

  /**
   * Add an edge to the graph connecting two existing nodes.
   * For undirected graphs, this also adds the inverse edge.
   */
  void add_edge(const GraphNodePtr &from_node, const GraphNodePtr &to_node, const EdgeData &edge_data) {
    using namespace std;
    check_has_node(from_node);
    check_has_node(to_node);
    check_no_edge(from_node, to_node);

    m_nodes_accessible_from.emplace(from_node, to_node);
    m_nodes_linking_to.emplace(to_node, from_node);
    auto edge_data_ptr = make_shared<EdgeData>(edge_data);
    m_edges.emplace(make_pair(from_node, to_node), edge_data_ptr);
  }

  /**
   * Remove an edge from the graph. If the graph is directed it will explicitly
   * remove only an edge from from_node to to_node.
   * <p/>
   */
  void remove_edge(const GraphNodePtr &from_node, const GraphNodePtr &to_node) {
    check_has_edge(from_node, to_node);

    // If the graph is directed I must remove from->to
    using namespace std;
    bool deleted_forward_edge = false;
    auto key_range = m_nodes_accessible_from.equal_range(from_node);
    for (auto iter = key_range.first; iter != key_range.second; ++iter) {
      if (iter->second == to_node) {
        m_nodes_accessible_from.erase(iter);
        deleted_forward_edge = true;
        break;
      }
    }

    // Remove inbounds
    key_range = m_nodes_linking_to.equal_range(to_node);
    for (auto iter = key_range.first; iter != key_range.second; ++iter) {
      if (iter->second == from_node) {
        m_nodes_accessible_from.erase(iter);
        break;
      }
    }

    // deleted forward edge is true iff
    //   this is a directed graph OR
    //   it's undirected and we hit on the right direction
    if (deleted_forward_edge) {
      m_edges.erase({from_node, to_node});
      return;
    }

    // Since we're here, it's an undirected graph
    assert(m_is_directed == false);

    // Search in the opposite direction
    key_range = m_nodes_accessible_from.equal_range(to_node);
    for (auto iter = key_range.first; iter != key_range.second; ++iter) {
      if (iter->second == from_node) {
        m_nodes_accessible_from.erase(iter);
        break;
      }
    }

    // Remove inbounds
    key_range = m_nodes_linking_to.equal_range(from_node);
    for (auto iter = key_range.first; iter != key_range.second; ++iter) {
      if (iter->second == to_node) {
        m_nodes_accessible_from.erase(iter);
        break;
      }
    }
    m_edges.erase({to_node, from_node});
  }
private:
  void add_new_edges(const std::map<GraphNodePtr, std::shared_ptr<EdgeData>> &from_node_edges,
                     const GraphNodePtr to_node,
                     const GraphNodePtr exclude_node,
                     std::function<EdgeData(const NodeData &,
                                            const NodeData &,
                                            const EdgeData &)> edge_merge_function) {
    for (const auto &node_edge : from_node_edges) {
      // Unless it's from the second node
      if (node_edge.first == exclude_node) {
        continue;
      }
      // Add new edge
      auto new_edge_data = edge_merge_function(node_edge.first->data(), to_node->data(), *(node_edge.second));
      add_edge(node_edge.first, to_node, new_edge_data);
    }
  }

  void add_new_edges(const GraphNodePtr from_node,
                     const std::map<GraphNodePtr, std::shared_ptr<EdgeData>> &to_node_edges,
                     const GraphNodePtr exclude_node,
                     std::function<EdgeData(const NodeData &,
                                            const NodeData &,
                                            const EdgeData &)> edge_merge_function) {
    for (const auto &node_edge : to_node_edges) {
      // Unless it's from the second node
      if (node_edge.first == exclude_node) {
        continue;
      }
      // Add new edge
      auto new_edge_data = edge_merge_function(from_node->data(), node_edge.first->data(), *(node_edge.second));
      add_edge(from_node, node_edge.first, new_edge_data);
    }
  }

  void collapse_directed_edge(const GraphNodePtr first_node,
                              const GraphNodePtr second_node,
                              std::function<NodeData(const NodeData &, const NodeData &)> node_merge_function,
                              std::function<EdgeData(const NodeData &,
                                                     const NodeData &,
                                                     const EdgeData &)> edge_merge_function) {
    using namespace std;

    // Generate a new node with the user defined callback
    auto new_node_data = node_merge_function(first_node->data(), second_node->data());
    auto new_node = add_node(new_node_data);

    // For each inbound edge to the first node, add a merged edge to this new node
    auto to_first_neighbours = nodes_with_edges_to(first_node);
    add_new_edges(to_first_neighbours, new_node, second_node, edge_merge_function);

    // For each inbound edge to the second node, add a merged edge to this new node
    auto to_second_neighbours = nodes_with_edges_to(second_node);
    add_new_edges(to_second_neighbours, new_node, first_node, edge_merge_function);

    // For each outbound edge from the first node, add a merged edge to this new node
    auto from_first_neighbours = nodes_with_edges_from(first_node);
    add_new_edges(new_node, from_first_neighbours, second_node, edge_merge_function);

    // For each outbound edge from the first node, add a merged edge to this new node
    auto from_second_neighbours = nodes_with_edges_from(second_node);
    add_new_edges(new_node, from_second_neighbours, first_node, edge_merge_function);

    // Remove the first and second nodes and any incident edges
    remove_node(first_node);
    remove_node(second_node);
  }

  void collapse_undirected_edge(const GraphNodePtr first_node,
                                const GraphNodePtr second_node,
                                std::function<NodeData(const NodeData &, const NodeData &)> node_merge_function,
                                std::function<EdgeData(const NodeData &,
                                                       const NodeData &,
                                                       const EdgeData &)> edge_merge_function) {
    using namespace std;
    auto to_first_neighbours = nodes_with_edges_to(first_node);
    auto to_second_neighbours = nodes_with_edges_to(second_node);

    // Undirected graph so we can short circuit here if there is only one incident edge for both
    // nodes (and it's implicitly the other node)
    if (to_first_neighbours.size() == 1 && to_second_neighbours.size() == 1) {
      spdlog::debug("Collapsing edge from {} to {} short circuit, removing",
                    fmt::ptr(first_node),
                    fmt::ptr(second_node));
      remove_edge(first_node, second_node);
      remove_node(first_node);
      remove_node(second_node);
      // No fix-up to do so we can just return.
      return;
    }

    // This will remove duplicates
    to_first_neighbours.insert(to_second_neighbours.begin(), to_second_neighbours.end());

    // Generate a new node with the user defined callback
    auto new_node_data = node_merge_function(first_node->data(), second_node->data());
    auto new_node = add_node(new_node_data);

    // Delete the old nodes (and incidentally any edges)
    remove_node(first_node);
    remove_node(second_node);

    // Add in new edges
    for (const auto &node_edge : to_first_neighbours) {
      if (node_edge.first == first_node || node_edge.first == second_node) {
        continue;
      }
      // Create a new edge from this node to the new node
      auto new_edge_data = edge_merge_function(node_edge.first->data(), new_node_data, *(node_edge.second));

      add_edge(node_edge.first, new_node, new_edge_data);
    }
  }

  /**
   * Check that NO edge exists and silently return or else throw
   * an exception. The edge is ordered.
   */
  void check_no_edge(const GraphNodePtr &first_node,
                     const GraphNodePtr &second_node) const {
    if (!has_edge(first_node, second_node)) {
      return;
    }
    std::ostringstream error_msg;
    error_msg << "Edge already exists from " << fmt::ptr(first_node)
              << " (" << first_node->data() << ")"
              << " to " << fmt::ptr(second_node)
              << " (" << second_node->data() << ")";
    auto msg = error_msg.str();
    spdlog::error(msg);
    throw std::runtime_error(msg);
  }
  /**
   * Check that an edge exists and silently return or else throw
   * an exception. The edge is ordered.
   */
  void check_has_edge(const GraphNodePtr &first_node,
                      const GraphNodePtr &second_node) const {
    if (has_edge(first_node, second_node)) {
      return;
    }
    std::ostringstream error_msg;
    error_msg << "No edge from " << fmt::ptr(first_node)
              << " (" << first_node->data() << ") to " << fmt::ptr(second_node)
              << " (" << second_node->data() << ")";
    auto msg = error_msg.str();
    spdlog::error(msg);
    throw std::runtime_error(msg);
  }

  /**
   * Check that a node exists and silently return or else throw
   * an exception.
   */
  void check_has_node(const GraphNodePtr &node) const {
    assert(node != nullptr);
    if (m_nodes.count(node) == 0) {
      std::ostringstream error_msg;
      error_msg << "No node " << fmt::ptr(node)
                << " (" << node->data() << ")";
      auto msg = error_msg.str();
      spdlog::error(msg);
      throw std::runtime_error(msg);
    }
  }

  /**
   * Check that NO node exists and silently return or else throw
   * an exception.
   */
  void check_no_node(const GraphNodePtr &node) const {
    assert(node != nullptr);

    // Check that node doesn't exist
    if (m_nodes.count(node) == 1) {
      std::ostringstream error_msg;
      error_msg << "Node " << fmt::ptr(node)
                << " (" << node->data() << ") already exists";
      auto msg = error_msg.str();
      spdlog::error(msg);
      throw std::runtime_error(msg);
    }
  }

public:
/**
 * Collapse an edge merging the two vertices that bound it into a single new vertex.
 * This is done by creating a new blended vertex (using a callback provided bu the client)
 * Adding this to the graph
 * Connecting all neighbours of the old end points to this node using an edge blending function
 * given by the user.
 * Deleting the original edge
 */
  void collapse_edge(const GraphNodePtr first_node,
                     const GraphNodePtr second_node,
                     std::function<NodeData(const NodeData &, const NodeData &)> node_merge_function,
                     std::function<EdgeData(const NodeData &,
                                            const NodeData &,
                                            const EdgeData &)> edge_merge_function) {
    using namespace std;
    check_has_node(first_node);
    check_has_node(second_node);
    check_has_edge(first_node, second_node);

    if (m_is_directed) {
      collapse_directed_edge(first_node, second_node, node_merge_function, edge_merge_function);
    } else {
      collapse_undirected_edge(first_node, second_node, node_merge_function, edge_merge_function);
    }
  }

/**
 * @return the number of nodes in this graph
 */
  inline size_t num_nodes() const { return m_nodes.size(); }

/**
 * @Return a vector of node values in the graph.
 */
  std::vector<NodeData> node_data() const {
    using namespace std;

    vector<NodeData> nodes;
    for (const auto &n : m_nodes) {
      nodes.push_back(n->data());
    }
    return nodes;
  }

/**
 *
 */
  std::vector<const GraphNodePtr> nodes() const {
    using namespace std;

    vector<const GraphNodePtr> nodes{begin(m_nodes), end(m_nodes)};
    return nodes;
  }

/**
 *
 */
  std::vector<Edge> edges() const {
    using namespace std;

    vector<Edge> edges;
    for (auto from_it = begin(m_edges); from_it != end(m_edges); ++from_it) {
      edges.emplace_back(from_it->first.first, from_it->first.second, from_it->second);
    }
    return edges;
  }

/**
 * Return the edge from from_node to to_node
 */
  std::shared_ptr<EdgeData> &edge(const GraphNodePtr &from_node, const GraphNodePtr &to_node) {
    check_has_edge(from_node, to_node);

    auto iter = m_edges.find({from_node, to_node});
    if (iter != m_edges.end()) {
      return iter->second;
    }
    iter = m_edges.find({to_node, from_node});
    return iter->second;
  }

/**
 * @return the number of edges in this graph
 */
  size_t num_edges() const {
    return m_edges.size();
  }

/**
 * @return a vector of the neighbours of a given node.
 * A neighbour is a node for which there is an edge from this node to that node.
 * In a directed graph nbr(a,b) does not imply nbr(b,a)
 * In an undirected graph these are equivalent.
 */
  std::vector<GraphNodePtr> neighbours(const GraphNodePtr &node) const {
    check_has_node(node);

    using namespace std;

    vector<GraphNodePtr> neighbours;
    // Neighbours are forward adjacent, ie you can get to them from node.
    auto key_range = m_nodes_accessible_from.equal_range(node);
    for (auto &map_iter = key_range.first; map_iter != key_range.second; ++map_iter) {
      neighbours.push_back(map_iter->second);
    }
    // In the event that the graph is not directed, we need to check for edges inbound to the node too
    // ad their other end is accessible
    if (!m_is_directed) {
      auto reverse_key_range = m_nodes_linking_to.equal_range(node);
      for (auto &map_iter = reverse_key_range.first; map_iter != reverse_key_range.second; ++map_iter) {
        neighbours.push_back(map_iter->second);
      }
    }
    return neighbours;
  }

/**
 * @return a map of neighbours for the given node along with the accompanying edge data.
 */
  std::map<GraphNodePtr, std::shared_ptr<EdgeData>> //
  nodes_with_edges_to(const GraphNodePtr &node) const {
    check_has_node(node);

    using namespace std;
    map<shared_ptr<GraphNode>, shared_ptr<EdgeData>> neighbours_with_edges;
    auto key_range = m_nodes_linking_to.equal_range(node);
    for (auto &map_entry = key_range.first; map_entry != key_range.second; ++map_entry) {
      const auto &nbr = map_entry->second;
      auto iter = m_edges.find({node, nbr});
      if (iter != end(m_edges)) {
        neighbours_with_edges.emplace(nbr, iter->second);
      } else {
        neighbours_with_edges.emplace(nbr, m_edges.at({nbr, node}));
      }
    }
    return neighbours_with_edges;
  }

/**
 * @return a map of neighbours for the given node along with the accompanying edge data.
 */
  std::map<GraphNodePtr, std::shared_ptr<EdgeData>>//
  nodes_with_edges_from(const GraphNodePtr &node) const {
    check_has_node(node);

    using namespace std;
    map<shared_ptr<GraphNode>, shared_ptr<EdgeData>> neighbours_with_edges;
    auto key_range = m_nodes_accessible_from.equal_range(node);
    for (auto &map_entry = key_range.first; map_entry != key_range.second; ++map_entry) {
      const auto &nbr = map_entry->second;
      auto iter = m_edges.find({node, nbr});
      if (iter != end(m_edges)) {
        neighbours_with_edges.emplace(nbr, iter->second);
      } else {
        neighbours_with_edges.emplace(nbr, m_edges.at({nbr, node}));
      }
    }
    return neighbours_with_edges;
  }

/**
 * @return true if there is an edge from node 1 to node 2. If the graph is undirected,
 * this will return true if there is an edge from node 2 to node 1.
 */
  bool has_edge(const GraphNodePtr &node_a, const GraphNodePtr &node_b) const {
    using namespace std;
    // If we have this edge return true
    if (m_edges.count({node_a, node_b}) == 1) {
      return true;
    }
    // If the graph is directed, and we don't have the requested edge, return false
    if (m_is_directed) {
      return false;
    }

    // Graph is undirected so if the reverse edge is present then it counts.
    return (m_edges.count({node_b, node_a}) == 1);
  }

/**
 * Return a vector of cycles in the graph..
 */
  std::vector<Path<GraphNodePtr>> cycles() const {
    using namespace std;

    vector<Path<shared_ptr<GraphNode>>> cycles;

    // For each node
    for (auto n : nodes()) {

      Path<GraphNodePtr> current_path;
      current_path.push_back(n);

      list<Path<GraphNodePtr>> paths;
      paths.push_back(current_path);

      bool done = false;

      // Explore all paths from node to find shortest cycle(s)
      while (!done) {
        // Grow each path by adding a neighbour not in the path
        list<Path<GraphNodePtr>> new_paths;
        for (const auto &path : paths) {
          auto node = path.last();

          auto next = neighbours(node);
          for (const auto &nbr : next) {
            if (!path.contains(nbr) || (path[0] == nbr && path.length() > 2)) {
              new_paths.emplace_back(path, nbr);
            }
          }
        }
        paths = new_paths;

        // Check if we have a cycle yet
        for (const auto &path : paths) {
          if (path.is_cycle()) {
            done = true;
            // Check whether a variant of this cycle is already known
            bool cycle_is_known = false;
            for (const auto &known_cycles : cycles) {
              if (path.is_equivalent_to(known_cycles)) {
                cycle_is_known = true;
                break;
              }
            }

            // If not, then add this cycle to my list
            if (!cycle_is_known) {
              cycles.push_back(path);
            }
          }
        }
      } // Consider next path
    }// Next node starting point
    vector<Path<GraphNodePtr>> cycle_vector;
    cycle_vector.assign(begin(cycles), end(cycles));
    return cycle_vector;
  }

private:
  bool m_is_directed;
// Set of all nodes
  std::set<GraphNodePtr> m_nodes;

// Which nodes are adjacent to a given node?
  std::multimap<GraphNodePtr, GraphNodePtr> m_nodes_accessible_from;
  std::multimap<GraphNodePtr, GraphNodePtr> m_nodes_linking_to;

// Store edge data for edge from A to B
  std::map<std::pair<GraphNodePtr, GraphNodePtr>, std::shared_ptr<EdgeData>> m_edges;
};
}
