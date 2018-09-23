#pragma once

namespace animesh {
  class Path;
}

namespace std {
  template <>
  struct hash<animesh::Path> {
    std::size_t operator()(const animesh::Path& path) const;
  };
}

namespace animesh {
  /* ********************************************************************************
  **                                                                            **
  ** GraphPath                                                                  **
  **                                                                            **
  ********************************************************************************/
  class Path {
  public:
    Path(){};
    Path(const std::vector<size_t>& nodes);
    Path(const Path& other_path);
    Path(const Path& other_path, size_t new_node);

    /**
    * @return The length of the path in nodes.
    */
    size_t length( ) const;

    /**
    * @return The first node index of the path.
    */
    size_t first() const;

    /**
    * @return The last node index of the path.
    */
    size_t last() const;

    /**
    * @return A Path with same nodes as this in reverse order.
    */
    Path reverse() const;

    /**
    * Strict equality operator.
    * Must be same nodes in same order
    */
    bool operator==(const Path &other) const;

    /**
    * Paths are equivalent iff:
    *  They are equal OR
    *  They are both cycles AND
    *     contain the same distinct nodes
    *     in the same order
    */
    bool is_equivalent_to( const Path& other ) const;

    /**
    * Paths are cycles if their first and last are the same.
    */
    bool is_cycle( ) const;

    /**
    * Push a new node to the path
    */
    void push_back( std::size_t node_idx );

    /**
    * @return the ith entry of the path.
    */
    std::size_t operator[](std::size_t idx ) const;

    /**
    * @return true if the path contains the given index
    */
    bool contains( std::size_t idx );

  private:
    std::vector<size_t>   m_node_indices;
  };
}
