#pragma once

#include "gtest/gtest.h"
#include <Graph/Graph.h>

class TestGraph : public ::testing::Test {
public:
    animesh::Graph<std::string, float>::GraphNode * gn1;
    animesh::Graph<std::string, float>::GraphNode * gn2;
    animesh::Graph<std::string, float> graph{true}; // directed
    animesh::Graph<std::string, float> undirected_graph; // undirected

	void SetUp( );
	void TearDown( );
};
