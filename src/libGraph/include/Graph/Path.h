#pragma once

//namespace animesh {
//    template<class PathNode>
//    class Path;
//}

//namespace std {
//  template <>
//  struct hash<animesh::Path<class PathNode>> {
//    std::size_t operator()(const animesh::Path& path) const;
//  };
//}


#include <vector>

namespace animesh {
    /* ********************************************************************************
    **                                                                            **
    ** GraphPath                                                                  **
    **                                                                            **
    ********************************************************************************/
    template<class PathNode>
    class Path {
    public:
        Path() = default;

        explicit Path(const std::vector<PathNode> &nodes) {
            m_nodes = nodes;
        }


        Path(const Path &other_path) {
            m_nodes = other_path.m_nodes;
        }

        Path(const Path &other_path, const PathNode &new_node) {
            m_nodes = other_path.m_nodes;
            m_nodes.push_back(new_node);
        }

        /**
        * @return The length of the path in nodes.
        */
        size_t length() const {
            return m_nodes.size();
        }

        /**
        * @return The first node index of the path.
        */
        const PathNode &first() const {
            return m_nodes.front();
        }

        /**
        * @return The last node index of the path.
        */
        const PathNode &last() const {
            return m_nodes.back();
        }


        /**
        * @return A Path with same nodes as this in reverse order.
        */
        Path reverse() const {
            Path reverse_path;
            for (auto it = m_nodes.rbegin(); it != m_nodes.rend(); ++it) {
                reverse_path.push_back(*it);
            }
            return reverse_path;
        }

        /**
        * Strict equality operator.
        * Must be same nodes in same order
        */
        bool operator==(const Path &other) const {
            return (m_nodes == other.m_nodes);
        }

        /**
        * Paths are cycles if their first and last are the same.
        */
        bool is_cycle() const {
            return first() == last();
        }

        /**
        * Push a new node to the path
        */
        void push_back(const PathNode &node) {
            m_nodes.push_back(node);
        }

        /**
        * @return the ith entry of the path.
        */
        const PathNode &operator[](std::size_t idx) const {
            return m_nodes[idx];
        }

        /**
        * @return true if the path contains the given index
        */
        bool contains(const PathNode &node) const {
            using namespace std;

            return find(begin(m_nodes), end(m_nodes), node) != end(m_nodes);
        }

        /**
* Paths are equivalent iff:
*  They are equal OR
*  They are both cycles AND
*     contain the same distinct nodes
*     in the same order
*/
        bool is_equivalent_to(const Path &other) const {
            using namespace std;
            if (*this == other) {
                return true;
            }

            if (m_nodes.size() != other.m_nodes.size()) {
                return false;
            }

            if (!is_cycle() || !other.is_cycle()) {
                return false;
            }

            // Handle single item case
            size_t num_items = m_nodes.size();
            if (num_items == 1) {
                return m_nodes[0] == other.m_nodes[0];
            }

            //  1,2,3,1  is equivalent to 2, 3, 1, 2
            // and 2,1,3,2
            // Find start of my list in other list
            auto this_first = m_nodes[0];
            auto it = find(begin(other.m_nodes), end(other.m_nodes), this_first);
            if (it == end(other.m_nodes)) {
                return false;
            }

            size_t other_first_idx = distance(begin(other.m_nodes), it);

            // Check forward
            bool matched = true;
            size_t other_idx = other_first_idx;
            for (size_t this_idx = 0; this_idx < num_items - 1; ++this_idx) {
                if (m_nodes[this_idx] != other.m_nodes[other_idx]) {
                    matched = false;
                    break;
                }
                other_idx = ((other_idx + 1) % (num_items-1));
            }

            if (!matched) {
                matched = true;
                size_t other_idx = other_first_idx;
                for (size_t this_idx = 0; this_idx < (num_items-1); ++this_idx) {
                    if (m_nodes[this_idx] != other.m_nodes[other_idx]) {
                        matched = false;
                        break;
                    }
                    other_idx = ((other_idx + num_items - 2) % (num_items - 1));
                }
            }

            return matched;
        }


    private:
        std::vector<PathNode> m_nodes;
    };
}
