#pragma once

#include <Eigen/Core>
#include <vector>
#include <gtest/gtest.h>
#include <Graph/Graph.h>
#include <Surfel/Surfel.h>
#include <Surfel/SurfelGraph.h>
#include <Surfel/SurfelBuilder.h>

class TestSurfel : public ::testing::Test {
public:
    SurfelBuilder * m_surfel_builder;

	void SetUp( );
	void TearDown();
};


class TestSurfelIO : public ::testing::Test {
public:
    void SetUp( );
    void TearDown();

protected:
    SurfelGraphPtr surfel_graph;
    SurfelBuilder * m_surfel_builder;
    std::mt19937 m_random_engine{123};
};

class TestSurfelGraph : public ::testing::Test {
public:
  void SetUp( );
  void TearDown();

protected:
  SurfelGraphPtr surfel_graph;
  SurfelBuilder * m_surfel_builder;
};
