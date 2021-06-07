#include "TestPoSy.h"
#include <PoSy/PoSy.h>
#include <Surfel/SurfelGraph.h>
#include <gmock/gmock.h>

const float EPSILON = 1e-4;

void TestPoSy::SetUp() {
    std::default_random_engine re{123};
    m_surfel_builder = new SurfelBuilder(re);
}

void TestPoSy::TearDown() {
    delete m_surfel_builder;
}

void expect_vector_equality(const Eigen::Vector3f &v1, const Eigen::Vector3f &v2) {
    EXPECT_FLOAT_EQ(v1.x(), v2.x());
    EXPECT_FLOAT_EQ(v1.y(), v2.y());
    EXPECT_FLOAT_EQ(v1.z(), v2.z());
}

SurfelGraphPtr
TestPoSy::makeTestGraph() {
    SurfelGraphPtr sg = std::make_shared<SurfelGraph>();

    m_surfel_builder
            ->reset()
            ->with_id("s1")
            ->with_tangent(1, 0, 0)
            ->with_reference_lattice_offset(0, 0);
    auto s1 = std::make_shared<Surfel>(m_surfel_builder->build());

    m_surfel_builder
            ->reset()
            ->with_id("s2")
            ->with_tangent(1, 0, 0)
            ->with_reference_lattice_offset(0, 0);
    auto s2 = std::make_shared<Surfel>(m_surfel_builder->build());
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
