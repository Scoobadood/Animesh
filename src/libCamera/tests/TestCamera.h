#pragma once

#include "gtest/gtest.h"

class TestCamera : public ::testing::Test {
public:
	void SetUp( );
	void TearDown();
};
