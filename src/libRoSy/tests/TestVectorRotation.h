#include "gtest/gtest.h"
#include <RoSy/RoSy.h>

#include <Eigen/Core>

#pragma once

class TestVectorRotation : public ::testing::Test {
public:
    Eigen::Vector3f zero{ 0.0f, 0.0f, 0.0f };
    Eigen::Vector3f unit_x{1.0f, 0.0f, 0.0f };
    Eigen::Vector3f unit_y{0.0f, 1.0f, 0.0f };
    Eigen::Vector3f unit_z{0.0f, 0.0f, 1.0f };
    Eigen::Vector3f vec_1_1_1{ 1.0f, 1.0f, 1.0f };

    Eigen::Vector3f vec_2_1_0{ 0.8f, 0.6f, 0.0f };
    Eigen::Vector3f vec_2_0_1{ 0.8f, 0.0f, 0.6f };
    Eigen::Vector3f vec_0_2_1{ 0.0f, 0.8f, 0.6f };

    void SetUp( );
    void TearDown();
};