#pragma once

#include "gtest/gtest.h"
#include <Graph/Graph.h>

class TestGraph : public ::testing::Test {

public:
    using GraphPtr = std::shared_ptr<typename animesh::Graph<std::string, float>>;
    using GraphNode = typename animesh::Graph<std::string, float>::GraphNode;
    using GraphNodePtr = std::shared_ptr<typename animesh::Graph<std::string, float>::GraphNode>;

    GraphNodePtr gn1;
    GraphNodePtr gn2;
    GraphPtr graph; // directed
    GraphPtr undirected_graph; // undirected

	void SetUp( );
	void TearDown( );
};
