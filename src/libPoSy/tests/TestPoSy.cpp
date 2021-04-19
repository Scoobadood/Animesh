#include "TestPoSy.h"
#include <PoSy/PoSy.h>
#include <Surfel/SurfelGraph.h>
#include <gmock/gmock.h>

void TestPoSy::SetUp() {}

void TestPoSy::TearDown() {}

void expect_vector_equality(const Eigen::Vector3f &v1, const Eigen::Vector3f &v2) {
    EXPECT_FLOAT_EQ(v1.x(), v2.x());
    EXPECT_FLOAT_EQ(v1.y(), v2.y());
    EXPECT_FLOAT_EQ(v1.z(), v2.z());
}

SurfelGraphPtr
TestPoSy::makeTestGraph() {
    SurfelGraphPtr sg = std::make_shared<SurfelGraph>();
    auto s1 = std::make_shared<Surfel>(Surfel("s1",
                                              {
                                                      {
                                                              {0, 0, 0},
                                                              10.0f,
                                                              Eigen::Matrix3f::Identity(),
                                                              {0.0f, 1.0f, 0.0f},
                                                              {0.0f, 0.0f, 0.0f}
                                                      }
                                              },
                                              {1.0f, 0.0f, 0.0f},
                                              {0.0f, 0.0f}));

    auto s2 = std::make_shared<Surfel>(Surfel("s2",
                                              {
                                                      {
                                                              {1, 1, 0},
                                                              10.0f,
                                                              Eigen::Matrix3f::Identity(),
                                                              {0.0f, 1.0f, 0.0f},
                                                              {0.5f, 0.5f, 0.0f}
                                                      }
                                              },
                                              {1.0f, 0.0f, 0.0f},
                                              {0.0f, 0.0f}
    ));
    const auto n1 = sg->add_node(s1);
    const auto n2 = sg->add_node(s2);
    sg->add_edge(n1, n2, SurfelGraphEdge{1.0f});
    return sg;
}


TEST_F(TestPoSy, TestTspWithPositiveHorizontalOffset) {
    using namespace std;
    auto p_prime = translate_4(
            {0.0f, 0.0f, 0.0f},
            {0.0f, 1.0f, 0.0f},
            {1.0f, 0.0f, 0.0f},
            {3, 0},
            1.0f);
    expect_vector_equality(p_prime, {3, 0, 0});
}

TEST_F(TestPoSy, TestTspWithPositiveVerticalOffset) {
    using namespace std;
    auto p_prime = translate_4(
            {0.0f, 0.0f, 0.0f},
            {0.0f, 1.0f, 0.0f},
            {0.0f, 0.0f, 1.0f},
            {1, 0},
            1.0f);
    expect_vector_equality(p_prime, {0, 0, 1});
}

TEST_F(TestPoSy, TestTspWithZeroOffset) {
    using namespace std;
    auto p_prime = translate_4(
            {1.0f, 2.0f, 3.0f},
            unit_x,
            unit_yz,
            {0, 0},
            1.0f);
    expect_vector_equality(p_prime, {1, 2, 3});
}

TEST_F(TestPoSy, TestTspWithNotUnitSpacing) {
    using namespace std;
    auto p_prime = translate_4(
            {1.0f, 2.0f, 3.0f},
            {1.0f, 0.0f, 0.0f},
            {0.0f, 1.0f, 0.0f},
            {1, 2},
            1.5f);
    expect_vector_equality(p_prime, {1, 3.5, 6});
}

TEST_F(TestPoSy, TestQijOnIntersectionOfPlanes1) {
    using namespace std;

    auto qij = compute_qij(
            {1, 0, 0},  // Point on XZ plane
            {0, 1, 0},
            {0, 1, 0}, // Point on YZ plane
            {1, 0, 0}
    );
    EXPECT_NEAR(qij[0], 0.0f, 1e-4);
    EXPECT_NEAR(qij[2], 0.0f, 1e-4);
    EXPECT_NEAR(qij[1], 0.0f, 1e-4);
}

TEST_F(TestPoSy, TestQijOnIntersectionOfPlanes2) {
    using namespace std;

    auto qij = compute_qij(
            {1, 0, 1}, // Point on XZ plane
            {0, 1, 0},
            {0, 1, 1}, // Point on YZ plane
            {1, 0, 0}
    );
    EXPECT_NEAR(qij[0], 0, 1e-4);
    EXPECT_NEAR(qij[2], 1, 1e-4);
    EXPECT_NEAR(qij[1], 0, 1e-4);
}

TEST_F(TestPoSy, TestQijOnIntersectionOfPlanes3) {
    using namespace std;

    auto qij = compute_qij(
            {2, 2, 0},  // Point on XY plane
            {0, 0, 1},
            {-1, 2, 0}, // Point on YZ plane
            {0, 0, 1}
    );
    EXPECT_NEAR(qij[0], 0.5, 1e-4);
    EXPECT_NEAR(qij[1], 2, 1e-4);
    EXPECT_NEAR(qij[2], 0, 1e-4);
}
/*********************************************************************************
 **
 **                  Test compute_lattice_neighbours
 **
 *********************************************************************************/

TEST_F(TestPoSy, TestSurroundingVerts1) {
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

TEST_F(TestPoSy, TestSurroundingVerts1a) {
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

TEST_F(TestPoSy, TestSurroundingVerts1b) {
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

TEST_F(TestPoSy, TestSurroundingVerts2) {
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

TEST_F(TestPoSy, TestRotatePjIntoN) {
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

/*********************************************************************************
 **
 **                                 Test Round_4
 **
 *********************************************************************************/
TEST_F(TestPoSy, RoundingSamePointAtOrigin) {
    using namespace std;
    auto r = round_4(
            unit_y,
            unit_x,
            origin,
            origin,
            1.0f);
    expect_vector_equality(r, {0, 0, 0});
}

TEST_F(TestPoSy, RoundingSamePointInPositiveSpace) {
    Eigen::Vector3f x{1, 2, 3};
    auto r = round_4(
            unit_y,
            unit_x,
            x,
            x,
            1.0f);
    expect_vector_equality(r, x);
}

TEST_F(TestPoSy, RoundingSamePointInNegativeSpace) {
    Eigen::Vector3f x{-1, -2, -3};
    auto r = round_4(
            unit_y,
            unit_x,
            x,
            x,
            1.0f);
    expect_vector_equality(r, x);
}


TEST_F(TestPoSy, RoundingPositiveFractionalValuesToOrigin) {
    Eigen::Vector3f x{1.25, 2, 3.5};
    auto r = round_4(
            unit_y,
            unit_x,
            origin,
            x,
            1.0f);
    expect_vector_equality(r, {1, 0, 4});
}

TEST_F(TestPoSy, RoundingPositiveFractionalValuesToArbitraryPoint) {
    Eigen::Vector3f p{1, 2, 3};
    Eigen::Vector3f x{1.25, 2, 3.5};
    auto r = round_4(
            unit_y,
            unit_x,
            p,
            x,
            1.0f);
    expect_vector_equality(r, {1, 2, 4});
}

TEST_F(TestPoSy, RoundingNegativeFractionalValuesToOrigin) {
    Eigen::Vector3f x{-1.25, -2, -3.5};
    auto r = round_4(
            unit_y,
            unit_x,
            origin,
            x,
            1.0f);
    expect_vector_equality(r, {-1, 0, -4});
}

TEST_F(TestPoSy, RoundingNegativeFractionalValuesToArbitraryPoint) {
    Eigen::Vector3f p{1, 2, 3};
    Eigen::Vector3f x{-1.25, -2, -3.5};
    auto r = round_4(
            unit_y,
            unit_x,
            origin,
            x,
            1.0f);
    expect_vector_equality(r, {-1, 0, -4});
}
