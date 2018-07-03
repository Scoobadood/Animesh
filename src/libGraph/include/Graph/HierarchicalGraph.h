#pragma once

#include <map>
#include <vector>
#include <set>
#include <iostream>

namespace animesh {

/*
 * A Graph representation that can handle hierarchical graphs.
 * The Graph is constructed over a set of Nodes and Edges
 */
template <class NodeData, class EdgeData>
class Graph {
public:


    /* ********************************************************************************
       **                                                                            **
       ** GraphNode                                                                  **
       **                                                                            **
       ********************************************************************************/

    /**
     * A node in the graph
     */
    class GraphNode {
    public: 
        /**
         * Construct a GraphNode from data
         */
        GraphNode( const NodeData& data) : m_data{ data }, m_parent_node{nullptr} {}

        /**
         * Construct a GraphNode as the parent of two other nodes
         */
        GraphNode( GraphNode * node_a, GraphNode * node_b, std::function<NodeData(const NodeData&, const NodeData&)> node_merge_function) {
            if( node_a == nullptr ) throw std::invalid_argument( "first node may not be null" );
            if( node_b == nullptr ) throw std::invalid_argument( "second node may not be null" );

            m_data = node_merge_function( node_a->m_data, node_b->m_data );
            m_children.push_back( node_a );
            m_children.push_back( node_b );
            node_a->m_parent_node = this;
            node_b->m_parent_node = this;
            m_parent_node = nullptr;
        }

        /**
         * Construct a GraphNode as the parent of a single GraphNode
         */
        GraphNode( GraphNode * node ) {
            if( node == nullptr ) throw std::invalid_argument( "node may not be null" );

            m_data = node->m_data;
            m_children.push_back( node );
            m_parent_node = nullptr;
            node->m_parent_node = this;
        }

        inline NodeData data() const { return m_data; }

        inline GraphNode * parent( ) const { return m_parent_node; }

        inline const std::vector<GraphNode *>& children( ) const { return m_children; } 

        inline bool has_child( GraphNode * node ) const { 
            return (std::find( m_children.begin(), m_children.end(), node ) != m_children.end());
        }

    private:
        NodeData                    m_data;
        GraphNode *                 m_parent_node;
        std::vector<GraphNode* >    m_children;
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
        Edge( GraphNode *from_node, GraphNode * to_node, float weight, const EdgeData& edge_data ) {
            if ( from_node == nullptr ) throw std::invalid_argument( "from node may not be null" );
            if ( to_node == nullptr ) throw std::invalid_argument( "to node may not be null" );
            if ( weight < 0 ) throw std::invalid_argument( "weight must be positive" );

            m_from_node = from_node;
            m_to_node = to_node;
            m_weight = weight;
            m_edge_data = edge_data;
        }

        GraphNode * from_node( ) const { return m_from_node; }
        GraphNode * to_node( ) const { return m_to_node; }
        float weight( ) const { return m_weight; }
        EdgeData data( ) const { return m_edge_data; }

    private:
        GraphNode *     m_from_node;
        GraphNode *     m_to_node;
        float           m_weight;
        EdgeData        m_edge_data;
    };


    /* ********************************************************************************
       **                                                                            **
       ** Graph private methods                                                      **
       **                                                                            **
       ********************************************************************************/
private:
    GraphNode * create_parent_node( GraphNode * node_a, GraphNode * node_b ) {
        return new GraphNode( node_a, node_b, m_node_merge_function );
    }

    GraphNode * create_parent_node( GraphNode * node_a ) {
        return new GraphNode( node_a );
    }

    /**
     * @return true if neither end node of this edge has a parent in a higher level
     */
    inline bool edge_has_no_parent( const Edge* edge ) const {
        return ( edge->from_node()->parent() == nullptr ) && 
                ( edge->to_node()->parent() == nullptr );
    }

    /* ********************************************************************************
       **                                                                            **
       ** Graph public methods                                                       **
       **                                                                            **
       ********************************************************************************/
public:
    Graph( std::function<NodeData(const NodeData&, const NodeData&)> node_merge_function,
           std::function<NodeData(NodeData, NodeData)> node_propagate_function  ) {
        m_node_merge_function = node_merge_function;
        m_node_propagate_function = node_propagate_function;
        m_up_graph = nullptr;
        m_down_graph = nullptr;
    }


    /**
     * Add a node for the given data.
     * @param data The data to be added.
     * @return a pointer to the node added
     */
    GraphNode * add_node( const NodeData& data ) {
        GraphNode * gn = new GraphNode( data );
        add_node( gn );
        return gn;
    }

    /**
     * Add the given GraphNode so long as it doesn;t already exist
     * @param node The GraphNode
     * @return a pointer to the node added
     */
    GraphNode * add_node( GraphNode * node ) {
        if( std::find( m_nodes.begin(), m_nodes.end( ), node ) == m_nodes.end() ) {
            m_nodes.push_back( node );
            std::vector<GraphNode*> x;
            m_adjacency[node] = x;
            return node;
        }
        throw std::invalid_argument( "can't add node to graph when it's already there");
    }

    /**
     * Add an edge to the graph connecting two existing nodes
     */
    Edge * add_edge( GraphNode * from_node, GraphNode * to_node, float weight, const EdgeData& edge_data) {
        std::cout << "      add edge from " << from_node << " to " << to_node << std::endl;
        if ( from_node == nullptr ) throw std::invalid_argument( "from node may not be null" );
        if ( to_node == nullptr ) throw std::invalid_argument( "to node may not be null" );
        if ( weight < 0 ) throw std::invalid_argument( "weight must be positive" );
        if ( std::find( m_nodes.begin(), m_nodes.end(), from_node ) == m_nodes.end() ) throw std::invalid_argument( "from node is unknown" );
        if ( std::find( m_nodes.begin(), m_nodes.end(), to_node ) == m_nodes.end() ) throw std::invalid_argument( "to node is unknown" );

        // If edge exists already, don't make a new one (throw)
        if ( m_adjacency[from_node].size() != 0 ) {
            if( std::find( m_adjacency[from_node].begin(), m_adjacency[from_node].end(), to_node ) != m_adjacency[from_node].end() ) {
                throw std::invalid_argument( "can't insert duplicate edge");
            }
        }

        m_adjacency[from_node].push_back( to_node );

        Edge * edge = new Edge( from_node, to_node, weight, edge_data);
        m_edges.push_back( edge );

        return edge;
    }

    void remove_edge(  GraphNode * from_node, GraphNode * to_node) {
        if ( from_node == nullptr ) throw std::invalid_argument( "from node may not be null" );
        if ( to_node == nullptr ) throw std::invalid_argument( "to node may not be null" );
        if ( std::find( m_nodes.begin(), m_nodes.end(), from_node ) == m_nodes.end() ) throw std::invalid_argument( "from node is unknown" );
        if ( std::find( m_nodes.begin(), m_nodes.end(), to_node ) == m_nodes.end() ) throw std::invalid_argument( "to node is unknown" );

        // If edge does not exists (throw)
        if ( m_adjacency[from_node].size() != 0 ) {
            if( std::find( m_adjacency[from_node].begin(), m_adjacency[from_node].end(), to_node ) == m_adjacency[from_node].end() ) {
                throw std::invalid_argument( "no such edge");
            }
        }

        // TODO: Perform actual delete
        m_adjacency.erase(from_node);

        auto edge_iter = m_edges.begin();
        for ( ; edge_iter != m_edges.end(); ) {
            if( ( (*edge_iter)->from_node() == from_node ) &&
                ( (*edge_iter)->to_node() == to_node ) ) {

                edge_iter = m_edges.erase(edge_iter);
            } else {
                ++edge_iter;
            }
        }
    }

    /**
     * @return the number of nodes in this graph
     */
    size_t num_nodes( ) const { return m_nodes.size(); }

    /**
     * @return the number of edges in this graph
     */
    size_t num_edges( ) const { return m_edges.size(); }

    /**
     * @return a vector of the neghbours of a given node
     */
    std::vector<GraphNode *> neighbours( GraphNode * node )  {
        std::vector<GraphNode *> ret = m_adjacency[node];

        return ret;
    }

    /**
     * @return true if there is an edge from node 1 to node 2
     */
    bool has_edge( GraphNode * node_a, GraphNode * node_b ) {
        std::vector<GraphNode *> neighbours = m_adjacency[node_a];
        return (std::find( neighbours.begin(), neighbours.end(), node_b ) != neighbours.end());
    }

    /**
     * @return the nodes of this graph
     */
    std::vector<animesh::Graph<NodeData, EdgeData>::GraphNode *>& nodes( ) {
      return m_nodes;
    }

    /**
     * Perform start contraction on this graph. The resultant graph
     * is set as the parent of this graph and nodes are mapped to their
     * corresponding node in the parent graph.
     * @return A pointer to the simplified graph or nullptr if the graph could
     * not be simplified
     */
    Graph<NodeData, EdgeData> * simplify( ) {

        if( m_edges.size() == 0 )
            return nullptr;

        // Make the up graph
        m_up_graph = new animesh::Graph<NodeData, EdgeData>( m_node_merge_function, m_node_propagate_function );
        m_up_graph->m_down_graph = this;


        /*
            for each edge in the lower graph
                if neither end node has a parent in upper graph
                    create a new merged node, parent of both end nodes
                    add to upper graph
                end
            end
         */        
        for( auto edge_iter = m_edges.begin( ); edge_iter != m_edges.end(); ++edge_iter) {
            Edge * edge = *edge_iter;
            if( edge_has_no_parent(edge) ) {
                GraphNode * new_node = create_parent_node( edge->from_node(), edge->to_node() );
                m_up_graph->add_node( new_node );                
            }
        }


        /*
            for each node in this graph
                if node has no parent
                    create new single node, parent of this node
                    add to upper graph
                end
            end
        */
        for( auto node_iter = m_nodes.begin(); node_iter != m_nodes.end(); ++node_iter) {
            GraphNode * node = (*node_iter);

            if( node->parent() == nullptr ) {
                GraphNode * new_node = create_parent_node( node );
                m_up_graph->add_node( new_node );                
            }
        }

        // For each edge in lower level
        // Find parents of end nodes in upper level
        // Add edge between parents if not existing
        // otherwise possibly increase weight
        for( auto edge : m_edges ) {
            GraphNode * gn1 = edge->from_node()->parent();
            GraphNode * gn2 = edge->to_node()->parent();

            if( ( gn1 != gn2 ) && !m_up_graph->has_edge( gn1, gn2) ) {
                m_up_graph->add_edge( gn1, gn2, 1.0f, nullptr );
            }
        }

        return m_up_graph;
    }

    bool can_propagate_down( ) const {
        return (m_down_graph != nullptr);
    }


    Graph<NodeData, EdgeData> * propagate_down( ) {
        if( m_down_graph == nullptr )
            throw std::invalid_argument("can't propagate down");

        /*
            for each node in the graph
                for each child
                    compute the new child value given the parent
                end
            end
         */
        for( auto& node : m_nodes ) {
            for( auto&  child : node->children() ) {
                m_node_propagate_function( node->data(), child->data() );
            }
        }

        return m_down_graph;
    }

private:
    std::function<NodeData(const NodeData&, const NodeData&)> m_node_merge_function;
    std::function<NodeData(NodeData, NodeData)> m_node_propagate_function;
    std::vector<GraphNode *>    m_nodes;
    std::vector<Edge *>         m_edges;
    Graph *                     m_up_graph;
    Graph *                     m_down_graph;
    std::map<GraphNode *, std::vector<GraphNode*>>  m_adjacency;
};


}
