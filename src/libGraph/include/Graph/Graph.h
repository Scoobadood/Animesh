#pragma once

#include <map>
#include <vector>
#include <set>
#include <iostream>
#include <list>
#include <unordered_set>
#include <Graph/Path.h>

namespace animesh {

struct vector_hash {
  size_t operator()(const std::vector<size_t>& v) const {
    std::hash<int> hasher;
    size_t seed = 0;
    for (int i : v) {
      seed ^= hasher(i) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }
    return seed;
  }
};

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
  *
  */
  Graph( bool is_directed = false) {
    m_is_directed = is_directed;
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
  * Add the given GraphNode so long as it doesn't already exist
  * @param node The GraphNode
  * @return a pointer to the node added
  */
  GraphNode * add_node( GraphNode * node ) {
    assert( std::find( m_nodes.begin(), m_nodes.end( ), node ) == m_nodes.end() );

    m_nodes.push_back( node );
    m_node_indices.insert(std::make_pair(node, m_nodes.size() - 1));
    return node;
  }

  /**
  * Add an edge to the graph connecting two existing nodes
  */
  Edge * add_edge( GraphNode * from_node, GraphNode * to_node, float weight, const EdgeData& edge_data) {
    using namespace std;

    assert( from_node != nullptr );
    assert( to_node != nullptr );
    assert( weight >= 0 );
    assert( std::find( m_nodes.begin(), m_nodes.end(), from_node ) != m_nodes.end() );
    assert( std::find( m_nodes.begin(), m_nodes.end(), to_node ) != m_nodes.end() );
    assert( !has_edge( from_node, to_node));
    m_adjacency.insert( make_pair( from_node, to_node));
    // Undirected graphs we have symmetric adjacency.
    if ( !m_is_directed) {
      m_adjacency.insert( make_pair( to_node, from_node));
    }
    // But edges are quite specific. Where important, we can flip edges.
    Edge * edge = new Edge( from_node, to_node, weight, edge_data);
    m_edges.push_back( edge );

    return edge;
  }

  /**
  * Remove an edge from the graph. If the graph is directed it will explicitly
  * remove only an edge from from_node to to_node.
  * <p/>
  * If the Graph is undirected, it will remove any edge between from_node
  * and to_node.
  * <p/>
  * Similarly for adjacencies
  */
  void remove_edge(  GraphNode * from_node, GraphNode * to_node) {
    assert( from_node != nullptr );
    assert( to_node != nullptr );
    assert( std::find( m_nodes.begin(), m_nodes.end(), from_node ) != m_nodes.end() );
    assert( std::find( m_nodes.begin(), m_nodes.end(), to_node ) != m_nodes.end() );

    // Delete adjacency.
    auto range = equal_range (from_node);
    for ( auto it = range.first; it != range.second; ++it ) {
      if ( it->second == to_node) {
        m_adjacency.erase(it);
        break;
      }
    }
    if ( !m_is_directed) {
      auto range = equal_range (to_node);
      for ( auto it = range.first; it != range.second; ++it ) {
        if ( it->second == from_node) {
          m_adjacency.erase(it);
          break;
        }
      }
    }

    auto edge_iter = m_edges.begin();
    for ( ; edge_iter != m_edges.end(); ) {
      if (
        ((*edge_iter)->from_node() == from_node )
        && ((*edge_iter)->to_node() == to_node )) {
        edge_iter = m_edges.erase(edge_iter);
        break;
      } else if (
        !m_is_directed
        && ((*edge_iter)->from_node() == to_node )
        && ((*edge_iter)->to_node() == from_node )) {
        edge_iter = m_edges.erase(edge_iter);
        break;
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
    std::vector<GraphNode *> data;
    auto range = m_adjacency.equal_range (node);
    for ( auto it = range.first; it != range.second; ++it ) {
      data.push_back(it->second);
    }

    return data;
  }

  std::size_t index_of( const GraphNode * gn ) const {
    auto it = m_node_indices.find(const_cast<GraphNode*>(gn));
    assert( it != m_node_indices.end());
    return it->second;
  }

  /**
  * TODO: This is O(Kn) in neighbours and nodes and is very expensive.
  * Fix this in future.
  * @return a vector of the neghbours of a given node
  */
  std::vector<size_t> neighbour_indices( GraphNode * node ) const {
    using namespace std;

    vector<GraphNode *> nodes = neighbours(node);
    vector<size_t> indices;

    for( auto node: nodes ) {
      indices.push_back(index_of(node));
    }
    return indices;
  }

  /**
  * @return a vector of the neghbours' data for a given node
  */
  std::vector<NodeData> neighbours_data( const GraphNode * node ) const {
    std::vector<NodeData> data;
    auto range = m_adjacency.equal_range ( const_cast<GraphNode*>(node));
    for ( auto it = range.first; it != range.second; ++it ) {
      data.push_back(it->second->data());
    }

    return data;
  }

  /**
  * @return true if there is an edge from node 1 to node 2. If the grpah is undirected,
  * this will return true if there is an edge from node 2 to node 1.
  */
  bool has_edge( GraphNode * node_a, GraphNode * node_b ) {
    using namespace std;

    // Has edge if one of the mappings of node_a is node_b
    auto range = m_adjacency.equal_range (node_a);
    for ( auto it = range.first; it != range.second; ++it ) {
      if ( it->second == node_b)
        return true;
    }
    return false;
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
  const std::vector<GraphNode *>& nodes( ) const {
    return m_nodes;
  }

  /**
  * Return a vector of cycles in the graph..
  */
  std::vector<Path> cycles( ) const {
    using namespace std;

    vector<Path> cycles;

    // For each node
    for ( size_t node_idx = 0; node_idx < num_nodes(); ++node_idx) {

      Path path;
      path.push_back(node_idx);

      list<Path> paths;
      paths.push_back(path);

      bool done = false;

      // Explore all paths from node to find shortest cycle(s)
      while ( !done ) {
        // Grow each path by adding a neighbour not in the path
        list<Path> new_paths;
        for( auto path : paths ) {
          size_t last_node_idx = path.last();
          vector<size_t> neighbours = neighbour_indices(nodes()[last_node_idx]);
          for ( auto neighbour : neighbours ) {
            if(!path.contains( neighbour ) || (path[0] == neighbour && path.length() > 2)) {
              Path new_path{ path, neighbour};
              new_paths.push_back( new_path );
            }
          }
        }
        paths = new_paths;

        // Check if we have a cycle yet
        for( auto path : paths ) {
          // // TESTS
          // cout << "Considering path : ";
          // for( int i=0; i<path.length(); i++) { cout << path[i] << " ";}
          // // TESTS
          if( path.is_cycle()) {
            done = true;
            // cout << " -- it's a cycle";
            // Check whether a variant of this cycle is already known
            bool cycle_is_known = false;
            for ( auto known_cycles : cycles ) {
              if ( path.is_equivalent_to(known_cycles)) {
                cycle_is_known = true;
                break;
              }
            }

            // If not, then add this cycle to my list
            if ( !cycle_is_known) {
              cycles.push_back(path);
              // cout << " -- added";
            } else {
              // cout << " -- already known";
            }
          }
          // cout << endl;
        }
      } // Consider next path
    }// Next node starting point
    vector<Path> cycle_vector;
    cycle_vector.assign(cycles.begin(), cycles.end());
    return cycle_vector;
  }

private:
  std::vector<GraphNode *>                        m_nodes;
  std::vector<Edge *>                             m_edges;
  std::multimap<GraphNode*, GraphNode*>           m_adjacency;
  std::map<GraphNode*, size_t>                    m_node_indices;
  bool                                            m_is_directed;
};
}
