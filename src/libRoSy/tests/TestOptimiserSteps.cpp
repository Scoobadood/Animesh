//
// Created by Dave Durbin (Old) on 2/8/21.
//

#include <Geom/Geom.h>
#include "TestOptimiserSteps.h"

void TestOptimiserSteps::SetUp() {}

void TestOptimiserSteps::TearDown() {}

/*
 * Test the function to return k,l that minimise the inter vector angle
 */
/* ********************************************************************************
 * ** Test computing the smallest rotations
 * ********************************************************************************/

TEST_F(TestOptimiserSteps, SmoothnessCalcsFor90) {
  using namespace Eigen;

  float theta;
  theta = degrees_angle_between_vectors(vec_1_0_0, vector_by_rotating_around_n(vec_1_0_0, vec_0_1_0, 0));
  EXPECT_FLOAT_EQ(theta, 0.0f);
  theta = degrees_angle_between_vectors(vec_1_0_0, vector_by_rotating_around_n(vec_1_0_0, vec_0_1_0, 1));
  EXPECT_FLOAT_EQ(theta, 90.0f);
  theta = degrees_angle_between_vectors(vec_1_0_0, vector_by_rotating_around_n(vec_1_0_0, vec_0_1_0, 2));
  EXPECT_FLOAT_EQ(theta, 180.0f);
  theta = degrees_angle_between_vectors(vec_1_0_0, vector_by_rotating_around_n(vec_1_0_0, vec_0_1_0, 3));
  EXPECT_FLOAT_EQ(theta, 90.0f);
}

TEST_F(TestOptimiserSteps, SmoothnessCalcsFor10) {
  using namespace Eigen;

  auto alpha = 10*M_PI/180.0f;

  Vector3f v{ std::cosf(alpha), 0.0f, std::sinf(alpha)};
  float theta;
  theta = degrees_angle_between_vectors(vec_1_0_0, vector_by_rotating_around_n(v, vec_0_1_0, 0));
  EXPECT_FLOAT_EQ(theta, 10.0f);
  theta = degrees_angle_between_vectors(vec_1_0_0, vector_by_rotating_around_n(v, vec_0_1_0, 1));
  EXPECT_FLOAT_EQ(theta, 80.0f);
  theta = degrees_angle_between_vectors(vec_1_0_0, vector_by_rotating_around_n(v, vec_0_1_0, 2));
  EXPECT_FLOAT_EQ(theta, 170.0f);
  theta = degrees_angle_between_vectors(vec_1_0_0, vector_by_rotating_around_n(v, vec_0_1_0, 3));
  EXPECT_FLOAT_EQ(theta, 100.0f);
}




