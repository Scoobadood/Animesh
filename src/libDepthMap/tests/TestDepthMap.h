#pragma once

#include "gtest/gtest.h"
#include <DepthMap/DepthMap.h>

class TestDepthMap : public ::testing::Test {
public:
	void SetUp( );
	void TearDown();

	Camera camera;

	unsigned int count_non_zero_cells(DepthMap& d);
};
