#pragma once

#include <Eigen/Core>
#include <Graph/Graph.h>
#include "Surfel.h"

class SurfelGraphEdge {
    float m_weight;
    std::vector<std::pair<unsigned short, unsigned short>> rosy_ij;
    // Best RoSy angles
    std::vector<unsigned short> m_k_ij;
    std::vector<unsigned short> m_k_ji;
    Eigen::Vector2i posy_tij;
    Eigen::Vector2i posy_tji;

public:
    SurfelGraphEdge( float weight ) :
            m_weight{weight},
            posy_tij{Eigen::Vector2i::Zero()},
            posy_tji{Eigen::Vector2i::Zero()}
    {}

    inline unsigned short k_ij(unsigned int frame_index) {
        if( m_k_ij.size() <= frame_index ) {
            m_k_ij.resize(frame_index + 1);
            m_k_ij.at(frame_index) = 0;
        }
        return m_k_ij.at(frame_index);
    }

    inline unsigned short k_ji(unsigned int frame_index) {
        if( m_k_ji.size() <= frame_index ) {
            m_k_ji.resize(frame_index + 1);
            m_k_ji.at(frame_index) = 0;
        }
        return m_k_ji.at(frame_index);
    }

    inline void set_k_ij(unsigned int frame_index, unsigned int kij) {
        if( m_k_ij.size() <= frame_index ) {
            m_k_ij.resize(frame_index + 1);
        }
        m_k_ij.at(frame_index) = kij;
    }

    inline void set_k_ji(unsigned int frame_index, unsigned int kji) {
        if( m_k_ji.size() <= frame_index ) {
            m_k_ji.resize(frame_index + 1);
        }
        m_k_ji.at(frame_index) = kji;
    }

    inline float weight() const {
        return m_weight;
    }
};
using SurfelGraph = animesh::Graph<std::shared_ptr<Surfel>, SurfelGraphEdge>;
using SurfelGraphPtr = std::shared_ptr<animesh::Graph<std::shared_ptr<Surfel>, SurfelGraphEdge>>;
using SurfelGraphNodePtr = std::shared_ptr<animesh::Graph<std::shared_ptr<Surfel>, SurfelGraphEdge>::GraphNode>;
