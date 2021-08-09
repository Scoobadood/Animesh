#pragma once

#include <GeomFileUtils/ObjFileParser.h>
#include "gtest/gtest.h"

class TestObjFileParser : public ::testing::Test {
public:
    animesh::ObjFileParser parser;

	void SetUp( ) override;
	void TearDown( ) override;
};
