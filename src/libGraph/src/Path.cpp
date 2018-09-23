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

Path::Path(const std::vector<size_t>& nodes) {
  m_node_indices = nodes;
}

Path::Path(const Path& other_path){
  m_node_indices = other_path.m_node_indices;
}

Path::Path(const Path& other_path, size_t new_node) {
  m_node_indices = other_path.m_node_indices;
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
* @return A Path with same nodes as this ut in reverse order.
*/
Path Path::reverse() const {
  Path reverse_path;
  for( auto it = m_node_indices.rbegin(); it != m_node_indices.rend(); ++it ) {
    reverse_path.push_back(*it);
  }
  return reverse_path;
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
* Paths are equivalent iff:
*  They are equal OR
*  They are both cycles AND
*     contain the same distinct nodes
*     in the same order
*/
bool Path::is_equivalent_to( const Path& other ) const {
  using namespace std;
  if( *this == other ) {
    return true;
  }

  if( !is_cycle() || !other.is_cycle() ) {
    return false;
  }

  // Handle single item case
  size_t num_items = m_node_indices.size();
  if( num_items == 1 ) {
    return m_node_indices[0] == other.m_node_indices[0];
  }

  //  1,2,3,1  is equivalent to 2, 3, 1, 2
  // and 2,1,3,2
  // Find start of my list in other list
  size_t this_first = m_node_indices[0];
  auto it = find( begin(other.m_node_indices), end(other.m_node_indices), this_first);
  if( it == end( other.m_node_indices)) {
    return false;
  }

  size_t other_first_idx = distance(begin(other.m_node_indices), it);

  // Check forward
  bool matched = true;
  for ( int this_idx = 0, other_idx = other_first_idx; this_idx < num_items-1; ++this_idx, other_idx = ((other_idx + 1) % (num_items-1))  ) {
    if( m_node_indices[this_idx] != other.m_node_indices[other_idx]) {
      matched = false;
      break;
    }
  }

  if( !matched ) {
    matched = true;
    for ( int this_idx = num_items-1, other_idx = other_first_idx; this_idx >= 0; --this_idx, other_idx = ((other_idx + 1) % (num_items-1))) {
      if( m_node_indices[this_idx] != other.m_node_indices[other_idx]) {
        matched = false;
        break;
      }
    }
  }

  return matched;
}
