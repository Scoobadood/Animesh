#pragma once

#include "gtest/gtest.h"

class TestFieldData : public ::testing::Test {
public:
	void SetUp( ) override;
    void TearDown( ) override;
};