#pragma once

#include <map>
#include <vector>
#include <set>
#include <iostream>
#include <list>
#include <tuple>
#include <unordered_set>
#include <spdlog/spdlog.h>
#include <fmt/format.h>

#include "Path.h"

namespace animesh {

/**
* A Graph representation that can handle hierarchical graphs.
* The Graph is constructed over a set of Nodes and Edges
*/
    template<class NodeData, class EdgeData>
    class Graph {
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
            inline void set_data(const NodeData & data) { m_data = data;}

        private:
            /** The data held in this node */
            NodeData m_data;
        };
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
            Edge(const std::shared_ptr<GraphNode> from_node,
                 const std::shared_ptr<GraphNode> to_node,
                 const std::shared_ptr<EdgeData> edge_data)
                 : m_from_node{from_node},
                m_to_node{to_node},
                m_edge_data{edge_data} { } ;

            std::shared_ptr<GraphNode> from() const { return m_from_node; }

            std::shared_ptr<GraphNode> to() const { return m_to_node; }

            std::shared_ptr<EdgeData> data() const { return m_edge_data; }

        private:
            std::shared_ptr<GraphNode> m_from_node;
            std::shared_ptr<GraphNode> m_to_node;
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
        static std::shared_ptr<GraphNode> make_node( const NodeData& node_data) {
            return std::make_shared<GraphNode>(node_data);
        }

        /**
        * Add a node for the given data. Convenience method.
        * @param data The data to be added.
        */
        std::shared_ptr<GraphNode> add_node(const NodeData &data) {
            auto n = make_node(data);
            add_node(n);
            return n;
        }

        /**
        * Add a node for the given data.
        * @param data The data to be added.
        */
        void add_node(const std::shared_ptr<GraphNode>& node) {
            using namespace std;
            assert(node != nullptr);

            // Check that node doesn't exist
            if (m_adjacency.count(node) != 0) {
                spdlog::error("Ignoring attempt to add node to Graph, it's already there");
                return;
            }
            m_adjacency.emplace(node, list<shared_ptr<GraphNode>>{});
        }

        /**
         * Remove the given node (and any incident edges)
         */
        void remove_node(const std::shared_ptr<GraphNode>& node) {
            const auto neighbours = m_adjacency.at(node);
            for( const auto& neighbour : neighbours) {
                // Remove the forward edge
                m_edges.erase(make_pair(node, neighbour));
                if( !m_is_directed) {
                    // For an undirected graph, remove the reverse edge too
                    m_edges.erase(make_pair(neighbour, node));
                    // And remove node from adjacency of neighbour
                    auto neighbour_neghbours = m_adjacency.at(neighbour);
                    neighbour_neghbours.erase(std::remove(begin(neighbour_neghbours), end(neighbour_neghbours), node), end(neighbour_neghbours));
                }
            }
            m_adjacency.erase(node);
        }

        /**
         * Add an edge to the graph connecting two existing nodes.
         */
        void add_edge(const std::shared_ptr<GraphNode>& from_node,
                      const std::shared_ptr<GraphNode>& to_node,
                      const EdgeData &edge_data) {
            using namespace std;

            if (has_edge(from_node, to_node)) {
                spdlog::warn("Ignoring attempt to add duplicate edge from {:p} to {:p}", fmt::ptr(from_node), fmt::ptr(to_node));
                return;
            }

            auto edge_data_ptr = make_shared<EdgeData>(edge_data);
            m_adjacency.at(from_node).push_back(to_node);
            m_edges.emplace(make_pair(from_node, to_node), edge_data_ptr);
            if (!m_is_directed) {
                m_adjacency.at(to_node).push_back(from_node);
                m_edges.emplace(make_pair(to_node, from_node), edge_data_ptr);
            }
        }

        /**
        * Remove an edge from the graph. If the graph is directed it will explicitly
        * remove only an edge from from_node to to_node.
        * <p/>
        */
        void remove_edge(const std::shared_ptr<GraphNode>& from_node,
                         const std::shared_ptr<GraphNode>& to_node) {
            using namespace std;

            if (!has_edge(from_node, to_node)) {
                spdlog::error("Ignoring attempt to remove non-existent edge from {:x} to {:x}", fmt::ptr(from_node), fmt::ptr(to_node));
                return;
            }
            auto from_neighbours = m_adjacency.at(from_node);
            from_neighbours.erase(std::remove(begin(from_neighbours), end(from_neighbours), to_node), end(from_neighbours));
            m_edges.erase(make_pair(from_node, to_node));

            if (!m_is_directed) {
                auto to_neighbours = m_adjacency.at(to_node);
                to_neighbours.erase(std::remove(begin(to_neighbours), end(to_neighbours), from_node), end(to_neighbours));
                m_edges.erase(make_pair(to_node, from_node));
            }
        }

        /**
        * @return the number of nodes in this graph
        */
        size_t num_nodes() const { return m_adjacency.size(); }

        /**
         * @Return a vector of node values in the graph.
         */
        std::vector<NodeData> node_data() const {
            using namespace std;

            vector<NodeData> nodes;
            for (auto it = begin(m_adjacency); it != end(m_adjacency); ++it) {
                nodes.push_back(it->first->data());
            }
            return nodes;
        }


        /**
         *
         */
        std::vector<const std::shared_ptr<GraphNode>> nodes() const {
            using namespace std;

            vector<const shared_ptr<GraphNode>> nodes;
            for (auto it = begin(m_adjacency); it != end(m_adjacency); ++it) {
                nodes.push_back(it->first);
            }
            return nodes;
        }

        /**
         *
         */
        std::vector<Edge> edges() const {
            using namespace std;

            vector<Edge> edges;
            for (auto from_it = begin(m_edges); from_it != end(m_edges); ++from_it) {
                edges.emplace_back(
                        from_it->first.first,
                        from_it->first.second,
                        from_it->second);
            }
            return edges;
        }

        /**
         * Return the edge from from_nbode to to_node
         */
         std::shared_ptr<EdgeData>&
                 edge(const std::shared_ptr<GraphNode>& from_node,
                 const std::shared_ptr<GraphNode>& to_node) {
             return m_edges.at(std::make_pair(from_node, to_node));
         }

        /**
        * @return the number of edges in this graph
        */
        size_t num_edges() const {
            return (m_is_directed)
                   ? m_edges.size()
                   : m_edges.size() / 2;
        }

        /**
        * @return a vector of the neighbours of a given node
        */
        std::vector<std::shared_ptr<GraphNode>> neighbours(const std::shared_ptr<GraphNode>& node) const {
            using namespace std;
            assert( node != nullptr);

            vector<shared_ptr<GraphNode>> neighbours;
            for (const auto &n : m_adjacency.at(node)) {
                neighbours.push_back(n);
            }
            return neighbours;
        }

        /**
        * @return true if there is an edge from node 1 to node 2. If the graph is undirected,
        * this will return true if there is an edge from node 2 to node 1.
        */
        bool has_edge(const std::shared_ptr<GraphNode> &node_a,
                      const std::shared_ptr<GraphNode>& node_b) const {
            using namespace std;

            return (m_edges.count(make_pair(node_a, node_b)) != 0 );
        }

        /**
        * Return a vector of cycles in the graph..
        */
        std::vector<Path<std::shared_ptr<GraphNode>>> cycles() const {
            using namespace std;

            vector<Path<shared_ptr<GraphNode>>> cycles;

            // For each node
            for (auto n : nodes()) {

                Path<shared_ptr<GraphNode>> current_path;
                current_path.push_back(n);

                list<Path<shared_ptr<GraphNode>>> paths;
                paths.push_back(current_path);

                bool done = false;

                // Explore all paths from node to find shortest cycle(s)
                while (!done) {
                    // Grow each path by adding a neighbour not in the path
                    list<Path<shared_ptr<GraphNode>>> new_paths;
                    for (const auto &path : paths) {
                        auto node = path.last();
                        auto edges = m_adjacency.at(node);
                        for (auto neighbour_node : edges) {
                            if (!path.contains(neighbour_node) || (path[0] == neighbour_node && path.length() > 2)) {
                                new_paths.emplace_back(path, neighbour_node);
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
            vector<Path<shared_ptr<GraphNode>>> cycle_vector;
            cycle_vector.assign(begin(cycles), end(cycles));
            return cycle_vector;
        }

    private:
        bool m_is_directed;
        // Which nodes are adjacent to a given node?
        std::map<
                std::shared_ptr<GraphNode>,
                std::list<std::shared_ptr<GraphNode>>> m_adjacency;
        // Store edge data for edge from A to B
        std::map<
                std::pair<
                        std::shared_ptr<GraphNode>,
                        std::shared_ptr<GraphNode>>,
                std::shared_ptr<EdgeData>> m_edges;
    };
}
