#pragma once

#include "gtest/gtest.h"
#include "Field/PointNormal.h"
#include <Graph/Graph.h>

class TestGraphCycles : public ::testing::Test {
public:
    animesh::Graph<std::size_t,int> m_test_graph;
    animesh::Graph<animesh::PointNormal::Ptr,int> m_sphere10x10;
    animesh::Graph<animesh::PointNormal::Ptr,int> m_cube;

	void SetUp( );
	void TearDown( );
};
