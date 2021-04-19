#pragma once

#include <Eigen/Core>
#include <vector>
#include <Surfel/SurfelGraph.h>
#include "gtest/gtest.h"

class TestPoSy : public ::testing::Test {
public:
	Eigen::Vector3f unit_x{1.0f, 0.0f, 0.0f };
	Eigen::Vector3f unit_y{0.0f, 1.0f, 0.0f };
	Eigen::Vector3f unit_z{0.0f, 0.0f, 1.0f };
    Eigen::Vector3f origin{0.0f, 0.0f, 0.0f };

    Eigen::Vector3f unit_yz{0.0f, M_SQRT1_2, M_SQRT1_2 };

    Eigen::Vector3f vec_1_0_1{ 1.0f, 0.0f, 1.0f };

	void SetUp( ) override;
	void TearDown() override;
    SurfelGraphPtr makeTestGraph();
};

