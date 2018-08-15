#pragma once

#include <map>
#include <vector>
#include <set>
#include <iostream>

namespace animesh {

/**
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
     * A node in the graph.
     * Nodes are containers for the data they represent
     */
    class GraphNode {
    public: 
        /**
         * Construct a GraphNode from data
         */
        GraphNode( const NodeData& data) : m_data{ data } {}

        /**
         * Return the data from the node
         */
        inline NodeData data() const { return m_data; }

        /**
         * Set the data in the node
         */
        inline void set_data( const NodeData& data ) { m_data = data; }

    private:
        /** The data held in this node */
        NodeData  m_data;
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
       ** Graph public methods                                                       **
       **                                                                            **
       ********************************************************************************/
public:
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
        assert( from_node != nullptr );
        assert( to_node != nullptr );
        assert( weight >= 0 );
        assert( std::find( m_nodes.begin(), m_nodes.end(), from_node ) != m_nodes.end() );
        assert( std::find( m_nodes.begin(), m_nodes.end(), to_node ) != m_nodes.end() );
        assert( !has_edge( from_node, to_node));

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
    std::vector<GraphNode *> neighbours( GraphNode * node ) const {
        std::vector<GraphNode *> ret = m_adjacency.at(const_cast<GraphNode*>(node));

        return ret;
    }

    /**
     * @return a vector of the neghbours' data for a given node
     */
    std::vector<NodeData> neighbours_data( const GraphNode * node ) const {
        std::vector<NodeData> data;
        for( auto gn : m_adjacency.at(const_cast<GraphNode*>(node))) {
            data.push_back(gn->data());
        }

        return data;
    }

    /**
     * @return true if there is an edge from node 1 to node 2
     */
    bool has_edge( GraphNode * node_a, GraphNode * node_b ) {
        std::vector<GraphNode *> neighbours = m_adjacency.at(node_a);
        return (std::find( neighbours.begin(), neighbours.end(), node_b ) != neighbours.end());
    }

    /**
     * @return the edges
     */
    std::vector<Edge *>& edges( ) {
      return m_edges;
    }


    /**
     * @return the nodes of this graph
     */
    std::vector<GraphNode *>& nodes( )  {
      return m_nodes;
    }

private:
    std::vector<GraphNode *>                        m_nodes;
    std::vector<Edge *>                             m_edges;
    std::map<GraphNode *, std::vector<GraphNode*>>  m_adjacency;
};


}
