//
// Created by Dave Durbin (Old) on 24/5/21.
//

#pragma once

#include <string>
#include "gtest/gtest.h"
#include <Graph/Graph.h>
#include <Graph/GraphNodeSimplifier.h>

class TestGraphNodeSimplifier : public ::testing::Test {
public:
    using GraphPtr = std::shared_ptr<typename animesh::Graph<std::string, float>>;
    using GraphNode = typename animesh::Graph<std::string, float>::GraphNode;
    using GraphNodePtr = std::shared_ptr<typename animesh::Graph<std::string, float>::GraphNode>;

    GraphPtr graph_ptr;
    GraphNodePtr m_node_a;
    GraphNodePtr m_node_b;
    GraphNodePtr m_node_c;
    GraphNodePtr m_node_d;
    GraphNodePtr m_node_e;
    GraphNodePtr m_node_x;

    GraphNodeSimplifier<std::string, float> * m_simplifier;

    void SetUp() override;
    void TearDown() override;
};