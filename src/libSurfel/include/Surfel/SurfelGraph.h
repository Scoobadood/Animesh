#pragma once

#include <Eigen/Core>
#include <Graph/Graph.h>
#include "Surfel.h"

class SurfelGraphEdge {
    float m_weight;
    std::vector<std::pair<unsigned short, unsigned short>> rosy_ij;
    // Best RoSy angles
    unsigned short m_k_ij;
    unsigned short m_k_ji;
    // PoSy displacements
    std::vector<Eigen::Vector2i> m_t_ij;
    std::vector<Eigen::Vector2i> m_t_ji;

public:
    explicit SurfelGraphEdge(float weight) //
            : m_weight{weight} //
    {}

    inline unsigned short k_ij() const {
        return m_k_ij;
    }

    inline unsigned short k_ji() const {
        return m_k_ji;
    }

    inline size_t t_values() const {
        assert( m_t_ij.size() == m_t_ji.size());
        return m_t_ij.size();
    }

    inline void set_k_ij(unsigned int kij) {
        m_k_ij = kij;
    }

    inline void set_k_ji(unsigned int kji) {
        m_k_ji = kji;
    }

    inline const Eigen::Vector2i& t_ji(unsigned int frame_index) {
        if (m_t_ji.size() <= frame_index) {
            m_t_ji.resize(frame_index + 1);
            m_t_ji.at(frame_index) = Eigen::Vector2i{0,0};
        }
        return m_t_ji.at(frame_index);
    }

    inline const Eigen::Vector2i& t_ij(unsigned int frame_index) {
        if (m_t_ij.size() <= frame_index) {
            m_t_ij.resize(frame_index + 1);
            m_t_ij.at(frame_index) = Eigen::Vector2i{0,0};
        }
        return m_t_ij.at(frame_index);
    }

    inline void set_t_ij(unsigned int frame_index, int x, int y ) {
        if (m_t_ij.size() <= frame_index) {
            m_t_ij.resize(frame_index + 1);
        }
        m_t_ij.at(frame_index) = Eigen::Vector2i {x,y};
    }

    inline void set_t_ji(unsigned int frame_index, int x, int y ) {
        if (m_t_ji.size() <= frame_index) {
            m_t_ji.resize(frame_index + 1);
        }
        m_t_ji.at(frame_index) = Eigen::Vector2i {x,y};
    }

    inline float weight() const {
        return m_weight;
    }
};

using SurfelGraph = animesh::Graph<std::shared_ptr<Surfel>, SurfelGraphEdge>;
using SurfelGraphPtr = std::shared_ptr<animesh::Graph<std::shared_ptr<Surfel>, SurfelGraphEdge>>;
using SurfelGraphNodePtr = std::shared_ptr<animesh::Graph<std::shared_ptr<Surfel>, SurfelGraphEdge>::GraphNode>;

unsigned int
get_num_frames(const SurfelGraphPtr &surfel_graph);

std::vector<SurfelGraphNodePtr>
get_node_neighbours_in_frame(
        const SurfelGraphPtr &graph,
        const SurfelGraphNodePtr &node_ptr,
        unsigned int frame_index);

std::ostream& operator<<( std::ostream& output, const std::shared_ptr<Surfel>& surfel );