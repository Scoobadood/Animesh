//
// Created by Dave Durbin (Old) on 7/6/21.
//

#pragma once
#include <Eigen/Core>
#include <vector>
#include "gtest/gtest.h"

class TestPoSyLatticeNeighbours  : public ::testing::Test {
protected:
    Eigen::Vector3f unit_x{1.0f, 0.0f, 0.0f };
    Eigen::Vector3f unit_y{0.0f, 1.0f, 0.0f };
    Eigen::Vector3f unit_z{0.0f, 0.0f, 1.0f };
    Eigen::Vector3f origin{0.0f, 0.0f, 0.0f };

    Eigen::Vector3f unit_yz{0.0f, M_SQRT1_2, M_SQRT1_2 };

    Eigen::Vector3f vec_1_0_1{ 1.0f, 0.0f, 1.0f };

public:
    void SetUp( ) override;
    void TearDown() override;
};

