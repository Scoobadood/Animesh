#pragma once

#include "gtest/gtest.h"
#include "Field/ObjFileParser.h"
#include <Graph/Graph.h>

class TestObjFileParser : public ::testing::Test {
public:
    animesh::ObjFileParser parser;

	void SetUp( );
	void TearDown( );
};
