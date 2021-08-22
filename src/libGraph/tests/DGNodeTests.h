//
// Created by Dave Durbin (Old) on 18/8/21.
//

#ifndef ANIMESH_LIBGRAPH_TESTS_DGNODETESTS_H
#define ANIMESH_LIBGRAPH_TESTS_DGNODETESTS_H

#include "gtest/gtest.h"
#include <Graph/Graph.h>

class DirectedGraphNodeTests : public ::testing::Test {
  using GraphPtr = std::shared_ptr<typename animesh::Graph<std::string, float>>;
  using GraphNode = typename animesh::Graph<std::string, float>::GraphNode;
  using GraphNodePtr = std::shared_ptr<typename animesh::Graph<std::string, float>::GraphNode>;

  // directed

  void SetUp();
  void TearDown();
protected:
  GraphPtr graph;
  GraphNodePtr gn1;
  GraphNodePtr gn2;
};

#endif //ANIMESH_LIBGRAPH_TESTS_DGNODETESTS_H
