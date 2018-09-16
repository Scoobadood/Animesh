#pragma once

#include "gtest/gtest.h"
#include <Graph/Graph.h>

class TestGraph : public ::testing::Test {
public:
    animesh::Graph<std::string, std::string>::GraphNode * gn1;
    animesh::Graph<std::string, std::string>::GraphNode * gn2;
    animesh::Graph<std::string, std::string> graph{true}; // directed
    animesh::Graph<std::string, std::string> undirected_graph; // undirected

	void SetUp( );
	void TearDown( );
};
