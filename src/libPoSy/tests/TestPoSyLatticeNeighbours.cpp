//
// Created by Dave Durbin (Old) on 7/6/21.
//

#include "TestPoSyLatticeNeighbours.h"
#include <PoSy/PoSy.h>
#include <gmock/gmock.h>

void TestPoSyLatticeNeighbours::SetUp(){}
void TestPoSyLatticeNeighbours::TearDown(){}

/*********************************************************************************
 **
 **                  Test compute_lattice_neighbours
 **
 *********************************************************************************/

TEST_F(TestPoSyLatticeNeighbours, InPositiveXYPlane) {
    using namespace std;

    auto s = compute_lattice_neighbours(
            {1, 0, 0.0},
            {3.5, 7.25, 0.0},
            unit_x,
            unit_y,
            1.0f);
    EXPECT_EQ(s.size(), 4);
    EXPECT_THAT(s, ::testing::UnorderedElementsAre(
            Eigen::Vector3f{3, 7, 0},
            Eigen::Vector3f{3, 8, 0},
            Eigen::Vector3f{4, 7, 0},
            Eigen::Vector3f{4, 8, 0}));
}

TEST_F(TestPoSyLatticeNeighbours, InPositiveXZPlane) {
    using namespace std;

    auto s = compute_lattice_neighbours(
            {1, 0, 0.0},
            {3.5, 0, 7.25},
            unit_x,
            unit_z,
            1.0f);
    EXPECT_EQ(s.size(), 4);
    EXPECT_THAT(s, ::testing::UnorderedElementsAre(
            Eigen::Vector3f{3, 0, 7},
            Eigen::Vector3f{3, 0, 8},
            Eigen::Vector3f{4, 0, 7},
            Eigen::Vector3f{4, 0, 8}));
}

TEST_F(TestPoSyLatticeNeighbours, InPositiveYZPlane) {
    using namespace std;

    auto s = compute_lattice_neighbours(
            {0, 1, 0.0},
            {0, 3.5, 7.25},
            unit_y,
            unit_z,
            1.0f);
    EXPECT_EQ(s.size(), 4);
    EXPECT_THAT(s, ::testing::UnorderedElementsAre(
            Eigen::Vector3f{0, 3, 7},
            Eigen::Vector3f{0, 3, 8},
            Eigen::Vector3f{0, 4, 7},
            Eigen::Vector3f{0, 4, 8}));
}

TEST_F(TestPoSyLatticeNeighbours, InXYPlaneInverted) {
    using namespace std;

    auto s = compute_lattice_neighbours(
            {1, 0, 0.0},
            {3.5, 7.25, 0.0},
            unit_x,
            -unit_y,
            1.0f);
    EXPECT_EQ(s.size(), 4);
    EXPECT_THAT(s, ::testing::UnorderedElementsAre(
            Eigen::Vector3f{3, 7, 0},
            Eigen::Vector3f{3, 8, 0},
            Eigen::Vector3f{4, 7, 0},
            Eigen::Vector3f{4, 8, 0}));
}

TEST_F(TestPoSyLatticeNeighbours, InXYPlaneWithNegTangent) {
    using namespace std;

    auto s = compute_lattice_neighbours(
            {1, 0, 0.0},
            {3.5, 7.25, 0.0},
            {-1.0, 0.0, 0.0},
            {0.0, 1.0, 0.0},
            1.0f);
    EXPECT_EQ(s.size(), 4);
    EXPECT_THAT(s, ::testing::UnorderedElementsAre(
            Eigen::Vector3f{3, 7, 0},
            Eigen::Vector3f{3, 8, 0},
            Eigen::Vector3f{4, 7, 0},
            Eigen::Vector3f{4, 8, 0}));
}

TEST_F(TestPoSyLatticeNeighbours, TestSurroundingVerts2) {
    using namespace std;

    auto s = compute_lattice_neighbours(
            {-23, 0, 0.0},
            {3.5, 7.25, 0.0},
            {-1.0, 0.0, 0.0},
            {0.0, -1.0, 0.0},
            1.0f);
    EXPECT_EQ(s.size(), 4);
    EXPECT_THAT(s, ::testing::UnorderedElementsAre(
            Eigen::Vector3f{3, 7, 0},
            Eigen::Vector3f{3, 8, 0},
            Eigen::Vector3f{4, 7, 0},
            Eigen::Vector3f{4, 8, 0}));
}

TEST_F(TestPoSyLatticeNeighbours, NotUnitRhoInXYPlane) {
    using namespace std;

    auto s = compute_lattice_neighbours(
            {1, 1, 0.0},
            {3.6, 7.25, 0.0},
            unit_x,
            unit_y,
            0.5f);
    EXPECT_EQ(s.size(), 4);
    EXPECT_THAT(s, ::testing::UnorderedElementsAre(
            Eigen::Vector3f{3.5, 7.0, 0.0},
            Eigen::Vector3f{3.5, 7.5, 0.0},
            Eigen::Vector3f{4.0, 7.0, 0.0},
            Eigen::Vector3f{4.0, 7.5, 0.0}));
}

TEST_F(TestPoSyLatticeNeighbours, NonIntegralNeighboursInXYPlane) {
    using namespace std;

    auto s = compute_lattice_neighbours(
            {1.5, 2.25, 0.0},
            {1.75, 7.25, 0},
            unit_x, unit_y,
            1.0f);
    EXPECT_EQ(s.size(), 4);
    EXPECT_THAT(s, ::testing::UnorderedElementsAre(
            Eigen::Vector3f{1.5, 7.25, 0},
            Eigen::Vector3f{1.5, 8.25, 0},
            Eigen::Vector3f{2.5, 7.25, 0},
            Eigen::Vector3f{2.5, 8.25, 0}));
}
