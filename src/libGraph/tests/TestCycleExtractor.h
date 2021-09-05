//
// Created by Dave Durbin (Old) on 30/8/21.
//

#pragma once
#include "gtest/gtest.h"
#include <Graph/Graph.h>


class TestCycleExtractor : public ::testing::Test {

public:
  using GraphPtr = std::shared_ptr<typename animesh::Graph<std::string, int>>;
  using GraphNode = typename animesh::Graph<std::string, int>::GraphNode;
  using GraphNodePtr = std::shared_ptr<typename animesh::Graph<std::string, int>::GraphNode>;

  GraphPtr graph; // directed
  GraphPtr undirected_graph; // undirected

  void SetUp( );
  void TearDown( );
};
