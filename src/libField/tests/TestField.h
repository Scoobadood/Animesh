#pragma once

#include "gtest/gtest.h"
#include <Eigen/Core>

class TestField : public ::testing::Test {
public:
    	void SetUp( ) override;
    	void TearDown( ) override;

        Eigen::Vector3f vec_0_0_0{ 0.0f, 0.0f, 0.0f };
        Eigen::Vector3f vec_1_1_1{ 1.0f, 1.0f, 1.0f };
        Eigen::Vector3f vec_1_0_0{ 1.0f, 0.0f, 0.0f };
        Eigen::Vector3f vec_0_1_0{ 1.0f, 1.0f, 2.0f };
        Eigen::Vector3f vec_0_0_1{ 1.0f, 1.0f, 2.0f };
};