//
// Created by Dave Durbin (Old) on 24/5/21.
//

#pragma once

#include <map>
#include <vector>
#include <random>

template<class NodeData, class EdgeData>
class GraphNodeSimplifier {
    using GraphPtr = std::shared_ptr<typename animesh::Graph<NodeData, EdgeData>>;
    using GraphNode = typename animesh::Graph<NodeData, EdgeData>::GraphNode;
    using GraphNodePtr = std::shared_ptr<typename animesh::Graph<NodeData, EdgeData>::GraphNode>;

public:
    GraphNodeSimplifier(
            std::function<NodeData (const std::vector<GraphNodePtr>&)> node_merge_function,
            std::function<EdgeData (const std::vector<EdgeData>&)> edge_merge_function
            )
            : m_node_merge_function(node_merge_function) //
            , m_edge_merge_function(edge_merge_function) //
            {}

    std::set<GraphNodePtr>
    neighbours_of(const GraphPtr &graph_ptr, const std::vector<GraphNodePtr> &source_nodes) {
        using namespace std;

        set<GraphNodePtr> neighbours_set;
        for( const auto& node : source_nodes ) {
            auto n2 = graph_ptr->neighbours(node);
            n2.erase(remove_if(begin(n2), end(n2), [&source_nodes](const GraphNodePtr& candidate_node){
                // return true if candidate_node is in source_nodes
                return (find(begin(source_nodes), end(source_nodes), candidate_node) != end(source_nodes));
            }), end(n2));
            neighbours_set.insert(begin(n2), end(n2));
        }
        return neighbours_set;
    }

    std::set<GraphNodePtr>
    neighbours_of(const GraphPtr &graph_ptr, const std::set<GraphNodePtr> &source_nodes) {
        using namespace std;

        set<GraphNodePtr> neighbours_set;
        for( const auto& node : source_nodes ) {
            auto n2 = graph_ptr->neighbours(node);
            n2.erase(remove_if(begin(n2), end(n2), [&source_nodes](const GraphNodePtr& candidate_node){
                // return true if candidate_node is in source_nodes
                std::cout << candidate_node->data() << std::endl;
                return (find(begin(source_nodes), end(source_nodes), candidate_node) != end(source_nodes));
            }), end(n2));
            neighbours_set.insert(begin(n2), end(n2));
        }
        return neighbours_set;
    }

    std::set<GraphNodePtr>
    second_neighbours_of(const GraphPtr &graph_ptr, const GraphNodePtr &source_node) {
        using namespace std;

        auto direct_neighbours = graph_ptr->neighbours(source_node);
        direct_neighbours.emplace_back(source_node);
        return neighbours_of(graph_ptr, direct_neighbours);
    }

    void
    collapse_node(
            const GraphPtr &graph_ptr,
            const GraphNodePtr &node_ptr,
            std::vector<const GraphNodePtr>& removed_nodes) {
        // Get neighbours
        auto neighbours = graph_ptr->neighbours(node_ptr);
        const auto neighbours2 = second_neighbours_of(graph_ptr, node_ptr);

        // Combine these into a new node
        neighbours.emplace_back(node_ptr);
        auto new_node_payload = m_node_merge_function(neighbours);
        auto new_node = graph_ptr->add_node(new_node_payload);

        // Add edges to new node
        for( const auto& n : neighbours2) {
            //TODO: Provide a list of actual edge data
            graph_ptr->add_edge(new_node, n, m_edge_merge_function(std::vector<EdgeData>{}));
        }

        // Delete old nodes
        for( const auto & n : neighbours) {
            graph_ptr->remove_node(n);
            removed_nodes.emplace_back(n);
        }
    }

    /* Simplify by:
     * Put all nodes into eligible set
     * Pick one at random
     * Loop:
     *   Generate super node
     *     This will remove nodes from the eligible set
     *   Compute furthest node in eligible set from the last picked node
     * : until There are no nodes in the eligible set.
     */
    GraphPtr
    simplify(const GraphPtr &graph_ptr, //
             std::map<GraphNodePtr, //
                     GraphNodePtr> &node_map, //
             std::function<float(const GraphNodePtr &, const GraphNodePtr &)> distance_function //
    ) {
        assert(graph_ptr->num_nodes() > 0);
        using namespace std;

        vector<GraphNodePtr> eligible_nodes;
        for( const auto& node : graph_ptr->nodes()) {
            eligible_nodes.emplace_back(node);
        }
        auto selected_node = select_random_node(eligible_nodes);

        vector<const GraphNodePtr> removed;
        do {
            removed.clear();
            collapse_node(graph_ptr, selected_node, removed);
            // Removed the collapsed nodes from the eligible nodes
            eligible_nodes.erase(remove_if(
                    begin(eligible_nodes),
                    end(eligible_nodes),
                    [&removed](const GraphNodePtr& node){

                    }
                    ), end(eligible_nodes));

        } while (!eligible_nodes.empty());
    }

private:
    GraphNodePtr select_random_node(const std::vector<GraphNodePtr> &eligible_nodes) {
        std::uniform_int_distribution<int> distribution(0, eligible_nodes.size());
        return eligible_nodes.at(distribution(m_random_engine));
    }

    GraphNodePtr select_furthest_node(
            const std::vector<GraphNodePtr> &eligible_nodes,
            const GraphNodePtr reference_node,
            std::function<float(const GraphNodePtr &p1, const GraphNodePtr &p2)> distance_function
    ) {
        float furthest_distance = MAXFLOAT;
        auto furthest_node = eligible_nodes.at(0);
        for (const auto &n : eligible_nodes) {
            auto d = distance_function(n, reference_node);
            if (d < furthest_distance) {
                furthest_distance = d;
                furthest_node = n;
            }
        }
        return furthest_node;
    }

private:
    std::default_random_engine m_random_engine{123};
    std::function<NodeData (const std::vector<GraphNodePtr>&)> m_node_merge_function;
    std::function<EdgeData (const std::vector<EdgeData>&)> m_edge_merge_function;
};
