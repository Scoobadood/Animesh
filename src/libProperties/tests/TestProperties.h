#pragma once

#include "gtest/gtest.h"

class TestProperties : public ::testing::Test {
public:
	void SetUp( );
	void TearDown();
};
