#pragma once

#include "gtest/gtest.h"
#include <Eigen/Core>

class TestField : public ::testing::Test {
public:
    	void SetUp( ) override;
    	void TearDown( ) override;

	    Eigen::Vector3f origin{ 0.0f, 0.0f, 0.0f };
    	Eigen::Vector3f normal{ 0.0f, 1.0f, 0.0f };
    	Eigen::Vector3f point_1_1_1{ 1.0f, 1.0f, 1.0f };
    	Eigen::Vector3f point_1_0_0{ 1.0f, 0.0f, 0.0f };
    	Eigen::Vector3f point_1_1_2{ 1.0f, 1.0f, 2.0f };
    	Eigen::Vector3f point_1_1_3{ 1.0f, 1.0f, 3.0f };
    	Eigen::Vector3f point_1_1_4{ 1.0f, 1.0f, 4.0f };
    	Eigen::Vector3f point_1_1_5{ 1.0f, 1.0f, 5.0f };
};