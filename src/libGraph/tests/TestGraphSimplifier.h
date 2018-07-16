#pragma once

#include "gtest/gtest.h"
#include <Graph/Graph.h>

std::string merge_strings( const std::string& s1, const std::string& s2 );
std::string propagate_strings( const std::string& s1, const std::string& s2 );

class TestGraphSimplifier : public ::testing::Test {
public:

    animesh::Graph<std::string, std::string>::GraphNode * gn1;
    animesh::Graph<std::string, std::string>::GraphNode * gn2;
	animesh::Graph<std::string, std::string> graph;


	void SetUp( );
	void TearDown( );
};