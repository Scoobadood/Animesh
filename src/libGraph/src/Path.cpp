#include <vector>
#include <Graph/Path.h>

using animesh::Path;

size_t std::hash<animesh::Path>::operator()(const animesh::Path& other_path) const {
  using namespace std;

  hash<size_t> hasher;
  size_t seed = 0;
  for (size_t i =0; i<other_path.length(); i++) {
    size_t node_idx = other_path[i];
    seed ^= hasher(node_idx) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
  }
  return seed;
}

  Path::Path(bool is_directed) {}
  Path::Path(const std::vector<size_t>& nodes, bool is_directed) {
      m_node_indices = nodes;
  }
  Path::Path(const Path& other_path){
    m_node_indices = other_path.m_node_indices;
    m_is_directed = other_path.m_is_directed;
  }
  Path::Path(const Path& other_path, size_t new_node) {
    m_node_indices = other_path.m_node_indices;
    m_is_directed = other_path.m_is_directed;
    m_node_indices.push_back( new_node);
  }

  /**
   * @return The lenght of the path in nodes.
   */
  size_t Path::length( ) const {
    return m_node_indices.size();
  }

  /**
   * @return The first node index of the path.
   */
  size_t Path::first() const {
    return m_node_indices.front();
  }

  /**
   * @return The last node index of the path.
   */
  size_t Path::last() const {
    return m_node_indices.back();
  }

  /**
   * Strict equality operator.
   * Must be same items in same order
   */
  bool Path::operator==(const Path &other) const {
    return (m_node_indices == other.m_node_indices);
  }
  /**
   * Paths are cycles if their first and last are the same.
   */
  bool Path::is_cycle( ) const {
    return first() == last();
  }

  /**
   * Push a new node to the path
   */
   void Path::push_back( std::size_t node_idx ) {
     m_node_indices.push_back( node_idx);
   }

   /**
    * @return the ith entry of the path.
    */
    std::size_t Path::operator[](std::size_t idx ) const {
      return m_node_indices[idx];
    }

    /**
     * @return true if the path contains the given index
     */
     bool Path::contains( std::size_t idx ) {
       using namespace std;

       return find( begin(m_node_indices), end(m_node_indices), idx ) != end(m_node_indices);
     }

  /**
   * Paths are equivalent if they are cycles and have the same nodes
   * in same order with some offset.
   */
  bool Path::is_equivalent_to( Path path2 ) const {
    // Not the same if not the same length
    if ( length() != path2.length() ) {
      return false;
    }

    size_t num_items = m_node_indices.size();
    size_t my_first = m_node_indices[0];
    size_t their_first_idx = 0;
    while (( their_first_idx < num_items) && (path2.m_node_indices[their_first_idx] != my_first) ) {
      their_first_idx++;
    }
    // If first item not found in ytheir list, fail
    if ( their_first_idx == num_items) {
      return false;
    }
    // If there's only one item, we match
    if ( num_items == 1 ) {
      return true;
    }

    int increment;
    if (m_node_indices[1] == path2.m_node_indices[(their_first_idx + 1) % num_items]) {
      // both paths seem ordered same way
      increment = 1;
    } else if (( !m_is_directed ) && (m_node_indices[1] == path2.m_node_indices[(their_first_idx + num_items - 1) % num_items])) {
      // Can be ordered down if not directed.
      increment = -1;
    } else {
      // Either the second element doesn't match or we're a directed path and can't be reversed.
      return false;
    }

    for ( int my_idx = 0, their_idx = their_first_idx; my_idx < num_items; my_idx++, their_idx += increment  ) {
      // if any corresponding elements don't match, fail
      if ( m_node_indices[my_idx] != path2.m_node_indices[(their_idx + num_items) % num_items]) {
        return false;
      }
    }
    // Must match
    return true;
  }

  /**
   * Canonicalise the path by placing lowest index first while maintaining the order.
   */
  Path Path::canonicalise() const {
    using namespace std;
    size_t lowest_idx_idx = 0;
    size_t lowest_idx = m_node_indices[lowest_idx_idx];
    for ( size_t idx = 1; idx < m_node_indices.size(); ++idx ) {
      if ( m_node_indices[idx] < lowest_idx ) {
        lowest_idx = m_node_indices[idx];
        lowest_idx_idx  = idx;
      }
    }
    vector<size_t> canonical_v;
    for ( size_t idx = 0; idx < m_node_indices.size(); ++idx ) {
      canonical_v.push_back( m_node_indices[ (lowest_idx_idx + idx) % m_node_indices.size() ] );
    }
    return Path{canonical_v};
  }
