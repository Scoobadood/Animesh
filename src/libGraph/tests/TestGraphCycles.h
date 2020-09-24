#pragma once

#include "gtest/gtest.h"
#include "Geom/Geom.h"
#include <Graph/Graph.h>

class TestGraphCycles : public ::testing::Test {
public:
  animesh::Graph<std::size_t,int> m_test_graph;
  
	void SetUp( );
	void TearDown( );
};
