#include <Graph/Graph.h>

GraphNode * Graph::add_node ( const void * data  ) {
    DataNodeMap::iterator itr = m_data_to_node_map.find(data);
    if (itr == m_data_to_node_map.end()) {
        GraphNode * gn = new GraphNode( data );
        m_data_to_node_map[data] = gn;
        return gn;
    }
    throw std::invalid_argument( "GraphNode already exists" );
}

/**
 * Add an edge to the graph connecting two existing nodes
 */
void Graph::add_edge( const void * from_data, const void * to_data, float weight, void * edge_data) {
    using namespace std;

    DataNodeMap::iterator from_itr = m_data_to_node_map.find(from_data);
    if( from_itr == m_data_to_node_map.end())
        throw std::invalid_argument( "Can't find from node in graph" );

    DataNodeMap::iterator to_itr = m_data_to_node_map.find(to_data);
    if( to_itr == m_data_to_node_map.end())
        throw std::invalid_argument( "Can't find to node in graph" );

    GraphNode * from_node  = from_itr->second;
    GraphNode * to_node    = to_itr->second;
    GraphNode::Edge edge   = make_tuple(weight, edge_data, to_node);
    from_node->m_edges.push_back(edge);
}
