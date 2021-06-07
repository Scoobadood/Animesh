//
// Created by Dave Durbin (Old) on 7/6/21.
//

#ifndef ANIMESH_TESTPOSYQIJ_H
#define ANIMESH_TESTPOSYQIJ_H

#pragma once

#include <Eigen/Core>
#include <vector>
#include <Surfel/SurfelGraph.h>
#include <Surfel/SurfelBuilder.h>
#include "gtest/gtest.h"

class TestPoSyQij : public ::testing::Test {
protected:
    Eigen::Vector3f unit_x{1.0f, 0.0f, 0.0f};
    Eigen::Vector3f unit_y{0.0f, 1.0f, 0.0f};
    Eigen::Vector3f unit_z{0.0f, 0.0f, 1.0f};

public:
    void SetUp( ) override;
    void TearDown() override;


};


#endif //ANIMESH_TESTPOSYQIJ_H
