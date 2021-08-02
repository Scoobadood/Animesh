//
// Created by Dave Durbin (Old) on 2/8/21.
//

#ifndef ANIMESH_TESTOPTIMISERSTEPS_H
#define ANIMESH_TESTOPTIMISERSTEPS_H

#include "gtest/gtest.h"
#include <Eigen/Core>

class TestOptimiserSteps : public ::testing::Test {
public:

  Eigen::Vector3f vec_1_0_0{1.0f, 0.0f, 0.0f};
  Eigen::Vector3f vec_0_1_0{0.0f, 1.0f, 0.0f};

  void SetUp();
  void TearDown();
};

#endif //ANIMESH_TESTOPTIMISERSTEPS_H
