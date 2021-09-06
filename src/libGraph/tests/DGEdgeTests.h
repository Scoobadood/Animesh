//
// Created by Dave Durbin (Old) on 19/8/21.
//

#ifndef ANIMESH_LIBGRAPH_TESTS_TEST_DATA_DGEDGETESTS_H
#define ANIMESH_LIBGRAPH_TESTS_TEST_DATA_DGEDGETESTS_H

#include "gtest/gtest.h"
#include <Graph/Graph.h>

class DirectedGraphEdgeTests  : public ::testing::Test {
public:
  using GraphPtr = std::shared_ptr<typename animesh::Graph<std::string, float>>;
  using GraphNode = typename animesh::Graph<std::string, float>::GraphNode;
  using GraphNodePtr = std::shared_ptr<typename animesh::Graph<std::string, float>::GraphNode>;

  static std::string to_string(const std::string& p) {
    return p;
  }

  // directed

  void SetUp();
  void TearDown();
protected:
  GraphPtr graph;
  GraphNodePtr gn1;
  GraphNodePtr gn2;
  GraphNodePtr gn3;
  GraphNodePtr gn4;
  void assertContainsInAnyOrder(
      const std::vector<const GraphNodePtr> & nodes,
      const std::vector<std::string> &expected);

};

#endif //ANIMESH_LIBGRAPH_TESTS_TEST_DATA_DGEDGETESTS_H