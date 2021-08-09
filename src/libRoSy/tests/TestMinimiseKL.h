#pragma once

#include <Eigen/Core>
#include "gtest/gtest.h"

class TestMinimiseKL : public ::testing::Test {
public:
	Eigen::Vector3f vec_1_0_0{ 1.0f, 0.0f, 0.0f };
	Eigen::Vector3f vec_0_1_0{ 0.0f, 1.0f, 0.0f };
	Eigen::Vector3f vec_0_0_1{ 0.0f, 0.0f, 1.0f };
	Eigen::Vector3f vec_0_1_R3{ 0.0f, 1.0f, sqrt( 3.0f ) };
	Eigen::Vector3f vec_0_R3_1{ 0.0f, sqrt( 3.0f ), 1.0f };

	void SetUp( );
	void TearDown();
};
