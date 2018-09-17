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
    Path(bool is_directed = false);
    Path(const std::vector<size_t>& nodes, bool is_directed = false);
    Path(const Path& other_path);
    Path(const Path& other_path, size_t new_node);
    /**
     * @return The lenght of the path in nodes.
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
     * Strict equality operator.
     * Must be same items in same order
     */
    bool operator==(const Path &other) const;

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

    /**
     * Paths are equivalent if they are cycles and have the same nodes
     * in same order with some offset.
     */
    bool is_equivalent_to( Path path2 ) const;

    /**
     * Canonicalise the path by placing lowest index first while maintaining the order.
     */
    Path canonicalise() const;

  private:
    std::vector<size_t>   m_node_indices;
    bool                  m_is_directed;
  };

}
