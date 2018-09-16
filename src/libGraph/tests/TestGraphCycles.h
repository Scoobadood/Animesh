#pragma once

#include "gtest/gtest.h"
#include <Graph/Graph.h>

class TestGraphCycles : public ::testing::Test {
public:
    animesh::Graph<std::size_t,int> test_graph;

	void SetUp( );
	void TearDown( );
};
