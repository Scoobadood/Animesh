#pragma once

#include <tuple>
#include <map>
#include <memory>
#include <Graph/Graph.h>

#define throw_invalid_argument(msg) \
    throw std::invalid_argument(msg " at " __FILE__ ":" + std::to_string(__LINE__))

namespace animesh {

/**
 * A GraphSimplifier extracts a simpler Graph from a base Graph. Mappins between the
 * Simpler and base graphs are maintained by the returned GraphMapping which is able to 
 * propagate changes from the source graph to the target
 * 
 * The Graph is constructed over a set of Nodes and Edges
 */
    template<class NodeData, class EdgeData>
    class GraphSimplifier {
        using GraphNodePtr = std::shared_ptr<typename animesh::Graph<NodeData, EdgeData>::GraphNode>;
        using GraphNode = typename animesh::Graph<NodeData, EdgeData>::GraphNode;
        using Graph = typename animesh::Graph<NodeData, EdgeData>;


    public:
        class GraphMapping {
        public:
            /**
             * Construct one
             * Propagate function takes parent and child nodes and generates new data for the child
             */
            GraphMapping(const std::function<NodeData(const NodeData &, const NodeData &)> &propagate_function,
                         const std::map<GraphNodePtr, GraphNodePtr> &child_to_parent)
                    : m_propagate_function{propagate_function},
                      m_child_to_parent{child_to_parent} {



                // Convert child->parent mapping to the other way around
                for (auto mapping : child_to_parent) {
                    auto x = m_parents_to_children.insert(std::make_pair(mapping.second, mapping.first));
                    if (x == m_parents_to_children.end()) throw std::runtime_error("Failed to insert");
                }
            }

            /**
             * Propagate changes in the parents to the children
             */
            void propagate() {
                /*
                    for each node in the graph
                        for each child
                            compute the new child value given the parent
                        end
                    end
                 */
                for (auto kvp : m_parents_to_children) {
                    NodeData new_data = m_propagate_function(kvp.first->data(), kvp.second->data());
//                    auto gn = const_cast<GraphNode*>(kvp.second);
                    (kvp.second)->set_data(new_data);
                }
            }

            /**
             * Return the childnodes of a node
             */
            std::vector<const GraphNodePtr> child_nodes(const GraphNodePtr gn) const {
                using namespace std;

                vector<const GraphNodePtr> children;

                auto ret = m_parents_to_children.equal_range(gn);
                for (auto it = ret.first; it != ret.second; ++it) {
                    children.push_back(it->second);
                }
                return children;
            }

            /**
             * Return the children of a node
             */
            std::vector<NodeData> children(const GraphNodePtr gn) const {
                using namespace std;

                vector<const GraphNodePtr> nodes = child_nodes(gn);
                vector<NodeData> children;
                for( auto node : nodes) {
                    children.push_back(node->data());
                }
                return children;
            }

            /**
             * Return the parent of a node
             */
            GraphNodePtr parent(const GraphNodePtr gn) const {
                using namespace std;

                return m_child_to_parent.at(gn);
            }

        private:
            /** function to propagate changes from parents to children */
            std::function<NodeData(const NodeData &, const NodeData &)> m_propagate_function;

            /** map from parents to children */
            std::multimap<GraphNodePtr, GraphNodePtr>                     m_parents_to_children;

            /** map from child to parents */
            std::map<GraphNodePtr, GraphNodePtr>                          m_child_to_parent;
        };


    private:
        /**
         * Construct a GraphNode as the parent of two other nodes
         */
        GraphNodePtr
        create_parent_node(const GraphNodePtr &node_a,
                           const GraphNodePtr &node_b,
                           std::map<GraphNodePtr, GraphNodePtr> &node_map) const {
            if (node_a == nullptr) throw_invalid_argument("first node may not be null");
            if (node_b == nullptr) throw_invalid_argument("second node may not be null");

            auto new_data = m_node_merge_function(node_a->data(), node_b->data());
            auto new_node = Graph::make_node(new_data);
            node_map.insert(std::make_pair(node_a, new_node));
            node_map.insert(std::make_pair(node_b, new_node));
            return new_node;
        }

        /**
         * Construct a GraphNode as the parent of one other nodes
         */
        GraphNodePtr
        create_parent_node( const GraphNodePtr node_a,
                std::map<GraphNodePtr, GraphNodePtr> &node_map) const {
            if (node_a == nullptr) throw std::invalid_argument("first node may not be null");

            auto new_node = Graph::make_node(node_a->data());
            node_map.emplace(node_a,new_node);
            return new_node;
        }

    public:

        /**
         * Construct with merge and propagate functions
         */
        GraphSimplifier(std::function<NodeData(const NodeData &, const NodeData &)> node_merge_function,
                        std::function<NodeData(const NodeData &, const NodeData &)> node_propagate_function) {
            m_node_merge_function = node_merge_function;
            m_node_propagate_function = node_propagate_function;
        }

        /**
         * Perform edge contraction on this graph. The resultant graph
         * is returned along with a mapper which maintains a link between the data in
         * the two graphs
         */
        std::pair<Graph *, GraphMapping> simplify(Graph *input_graph) const {
            using namespace std;

            // Can only simplify if the input graph has at least one edge
            if (input_graph->num_edges() == 0)
                throw invalid_argument("Can't simplify Graph with no edges");

            // Make the up graph
            auto *output_graph = new Graph();

            // Make the mapper
            map<GraphNodePtr, GraphNodePtr> node_map;

            /*
                for each edge in the input graph
                    if neither end node has a parent in output graph
                        create a new merged node, parent of both end nodes in output graph
                    end
                end
             */
            for (auto edge : input_graph->edges()) {
                if ((node_map.count(edge.from()) == 0)
                    && (node_map.count(edge.to()) == 0)) {

                    auto new_node = create_parent_node(edge.from(), edge.to(), node_map);
                    output_graph->add_node(new_node);
                }
            }


            /*
                for each node in input graph
                    if node has no parent in output graph
                        create new single node, parent of this node in output graph
                    end
                end
            */
            for (auto node : input_graph->nodes()) {
                if (node_map.count(node) == 0) {
                    auto new_node = create_parent_node(node, node_map);
                    output_graph->add_node(new_node);
                }
            }

            /*
                for each edge in the input graph
                    find parents of end nodes in output graph
                    if no edge exists between these nodes in output graph
                        add edge
                    else
                        increase weight of existing edge
                    end
                end
            */
            for (auto edge : input_graph->edges()) {
                auto gn1 = node_map.at(edge.from());
                auto gn2 = node_map.at(edge.to());

                if ((gn1 != gn2) && !output_graph->has_edge(gn1, gn2)) {
                    output_graph->add_edge(gn1, gn2, 1.0f);
                } else {
                    // ... increase weight
                }
            }

            GraphMapping mapping{m_node_propagate_function, node_map};

            return make_pair(output_graph, mapping);
        }

    private:
        std::function<NodeData(const NodeData &, const NodeData &)> m_node_merge_function;
        std::function<NodeData(const NodeData& , const NodeData& )> m_node_propagate_function;
    };
}