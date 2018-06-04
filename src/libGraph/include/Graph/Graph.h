#pragma once

#include <map>
#include <vector>

#include <Element/Element.h>

/**
 * A node in the graph
 */
template<class NodeData, class EdgeData>
struct GraphNode {
	// For each adjacent node we store cost, an edge payload and the node
	typedef std::tuple<float, EdgeData, GraphNode<NodeData, EdgeData>*> Edge;
    std::vector<Edge>	m_edges;// weight, edge data, next node
    const NodeData		m_data;

    GraphNode( const NodeData data) : m_data{ data } {}

    void add_edge(float weight, EdgeData edge_data, 
    	GraphNode<NodeData, EdgeData> * to_node) {
    	m_edges.push_back(std::make_tuple(weight, edge_data, to_node));
    }
};

/*
 * Graph is graph representing a 3D structure (point cloud, mesh, triangle soup.
 * Each element of the structure is represnted as a GraphNode which stores its location
 * and normal.
 * The Graph stores the relationship between GraphNodes and can return the neghbours of
 * an individual node.
 */

template <class NodeData, class EdgeData>
class Graph {
public:

	/**
	 * Add a node with the given payload
	 * @param data The data to be added. This should not exist in the graph already.
	 * We check this by doing a direct point comparison rather than a more sophisticated
	 * equality test.
	 */
	GraphNode<NodeData, EdgeData> * add_node( const NodeData data );

	/**
	 * Add an edge to the graph connecting two existing nodes
	 */
	void add_edge( const NodeData from_data, const NodeData to_data, float weight, EdgeData edge_data);

	/**  Map from data pointer to the node */
	typedef std::map<const NodeData, GraphNode<NodeData, EdgeData> *> DataNodeMap;
	DataNodeMap m_data_to_node_map;
};


template <class NodeData, class EdgeData>
GraphNode<NodeData, EdgeData> * Graph<NodeData, EdgeData>::add_node ( const NodeData data  ) {
    typename DataNodeMap::iterator itr = m_data_to_node_map.find(data);
    if (itr == m_data_to_node_map.end()) {
        GraphNode<NodeData, EdgeData> * gn = new GraphNode<NodeData, EdgeData>( data );
        m_data_to_node_map[data] = gn;
        return gn;
    }
    throw std::invalid_argument( "GraphNode already exists" );
}

/**
 * Add an edge to the graph connecting two existing nodes
 */
template <class NodeData, class EdgeData>
void Graph<NodeData, EdgeData>::add_edge( const NodeData from_data, const NodeData to_data, float weight, EdgeData edge_data) {
    using namespace std;

    typename DataNodeMap::iterator from_itr = m_data_to_node_map.find(from_data);
    if( from_itr == m_data_to_node_map.end())
        throw std::invalid_argument( "Can't find from node in graph" );

    typename DataNodeMap::iterator to_itr = m_data_to_node_map.find(to_data);
    if( to_itr == m_data_to_node_map.end())
        throw std::invalid_argument( "Can't find to node in graph" );

    GraphNode<NodeData, EdgeData> * from_node  = from_itr->second;
    GraphNode<NodeData, EdgeData> * to_node    = to_itr->second;
    from_node->add_edge(weight, edge_data, to_node);
}