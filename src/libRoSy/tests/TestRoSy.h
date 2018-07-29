#pragma once

#include <Eigen/Core>
#include <vector>
#include "gtest/gtest.h"

class TestRoSy : public ::testing::Test {
public:
	Eigen::Vector3f vec_1_0_0{ 1.0f, 0.0f, 0.0f };
	Eigen::Vector3f vec_0_1_0{ 0.0f, 1.0f, 0.0f };
	Eigen::Vector3f vec_0_0_1{ 0.0f, 0.0f, 1.0f };
    Eigen::Vector3f vec_1_0_1{ 1.0f, 0.0f, 1.0f };

	void SetUp( );
	void TearDown();
};
