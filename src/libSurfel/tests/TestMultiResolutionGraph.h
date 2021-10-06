//
// Created by Dave Durbin (Old) on 4/10/21.
//

#pragma once
#include <gtest/gtest.h>
#include <Surfel/SurfelGraph.h>

class TestMultiResolutionGraph  : public ::testing::Test {
public:
  void SetUp( );
  void TearDown();

protected:
  SurfelGraphPtr m_surfel_graph;
};