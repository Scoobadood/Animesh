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
	Eigen::Vector3f vec_1_0_0{ 1.0f, 0.0f, 0.0f };
	Eigen::Vector3f vec_0_1_0{ 0.0f, 1.0f, 0.0f };
	Eigen::Vector3f vec_0_0_1{ 0.0f, 0.0f, 1.0f };
    Eigen::Vector3f vec_1_0_1{ 1.0f, 0.0f, 1.0f };

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

};
