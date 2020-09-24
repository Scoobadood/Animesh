#include "TestPoSy.h"
#include <PoSy/PoSy.h>

void TestPoSy::SetUp( ) {}
void TestPoSy::TearDown( ) {}

void expect_vector_equality(const Eigen::Vector3f& v1, const Eigen::Vector3f& v2 ) {
    EXPECT_FLOAT_EQ(v1.x(), v2.x());
    EXPECT_FLOAT_EQ(v1.y(), v2.y());
    EXPECT_FLOAT_EQ(v1.z(), v2.z());
}

/* ********************************************************************************
 * *
 * *  Test average rosy vectors
 * *   
 * ********************************************************************************/
TEST_F(TestPoSy, ClosestPointsOne) {
    using namespace Eigen;

    const auto tuple = closest_points({Vector3f{0.0, 0.0, 0.0}},
                    {Vector3f{0.0, 0.0, 0.0}}
                    );
    EXPECT_EQ(std::get<0>(tuple), 0);
    EXPECT_EQ(std::get<1>(tuple), 0);
    EXPECT_FLOAT_EQ(std::get<4>(tuple), 0.0);
}

TEST_F(TestPoSy, ComputeLatticeXZ) {
    using namespace Eigen;

    const auto vertices = compute_local_lattice_vertices(
            Vector3f{0.0, 0.0, 0.0},
            Vector3f{1.0, 0.0, 0.0},
            Vector3f{0.0, 0.0, -1.0},
            1.5f);

    expect_vector_equality(vertices.at(0), Vector3f{0.0, 0.0, 0.0});
    expect_vector_equality(vertices.at(1), Vector3f{1.5, 0.0, 0.0});
    expect_vector_equality(vertices.at(2), Vector3f{1.5, 0.0, -1.5});
    expect_vector_equality(vertices.at(3), Vector3f{0.0, 0.0, -1.5});
    expect_vector_equality(vertices.at(4), Vector3f{-1.5, 0.0, -1.5});
    expect_vector_equality(vertices.at(5), Vector3f{-1.5, 0.0, 0.0});
    expect_vector_equality(vertices.at(6), Vector3f{-1.5, 0.0, 1.5});
    expect_vector_equality(vertices.at(7), Vector3f{-0.0, 0.0, 1.5});
    expect_vector_equality(vertices.at(8), Vector3f{1.5, 0.0, 1.5});
}

TEST_F(TestPoSy, ComputeLatticeYZ) {
    using namespace Eigen;

    const auto vertices = compute_local_lattice_vertices(
            Vector3f{0.0, 1.0, 1.0},
            Vector3f{0.0, 0.0, 1.0},
            Vector3f{0.0, -1.0, 0.0},
            2.0f);

    expect_vector_equality(vertices.at(0), Vector3f{0.0, 1.0, 1.0});
    expect_vector_equality(vertices.at(1), Vector3f{0.0, 1.0, 3.0});
    expect_vector_equality(vertices.at(2), Vector3f{0.0, -1.0, 3.0});
    expect_vector_equality(vertices.at(3), Vector3f{0.0, -1.0, 1.0});
    expect_vector_equality(vertices.at(4), Vector3f{0.0, -1.0, -1.0});
    expect_vector_equality(vertices.at(5), Vector3f{0.0, 1.0, -1.0});
    expect_vector_equality(vertices.at(6), Vector3f{0.0, 3.0, -1.0});
    expect_vector_equality(vertices.at(7), Vector3f{0.0, 3.0, 1.0});
    expect_vector_equality(vertices.at(8), Vector3f{0.0, 3.0, 3.0});
}

TEST_F(TestPoSy, ComputeLatticeXY) {
    using namespace Eigen;

    const auto vertices = compute_local_lattice_vertices(
            Vector3f{-1.0, -2.0, 0.0},
            Vector3f{1.0, 0.0, 0.0},
            Vector3f{0.0, 1.0, 0.0},
            2.5f);

    expect_vector_equality(vertices.at(0), Vector3f{-1.0, -2.0, 0.0});
    expect_vector_equality(vertices.at(1), Vector3f{1.5, -2.0, 0.0});
    expect_vector_equality(vertices.at(2), Vector3f{1.5, 0.5, 0.0});
    expect_vector_equality(vertices.at(3), Vector3f{-1.0, 0.5, 0.0});
    expect_vector_equality(vertices.at(4), Vector3f{-3.5, 0.5, 0.0});
    expect_vector_equality(vertices.at(5), Vector3f{-3.5, -2.0, 0.0});
    expect_vector_equality(vertices.at(6), Vector3f{-3.5, -4.5, 0.0});
    expect_vector_equality(vertices.at(7), Vector3f{-1.0, -4.5, 0.0});
    expect_vector_equality(vertices.at(8), Vector3f{1.5, -4.5, 0.0});
}


// Given two points in the plane, offset by rho/2, both claiming to be at their mesh vertex
// We expect that the negotiated coordinate will be the midpoint of the line between their respective estimates.
TEST_F(TestPoSy, SmoothInXZPlane) {
    using namespace Eigen;

    const float rho = 1.0f;
    // v1 representative vector assumes it's at origin and is
    const auto v1 = Vector3f{0.0, 0.0, 0.0};
    const auto p1 = v1 + Vector3f{0.0, 0.0, 0.0};
    const auto o1 = Vector3f{1.0, 0.0, 0.0};
    const auto n1 = Vector3f{0.0, 1.0, 0.0};

    // v2 representatiove vector assumes it's at origin and is actually at 1,0,1
    const auto v2 = Vector3f{0.5, 0.0, 0.0};
    const auto p2 = v2 + Vector3f{0.0, 0.0, 0.0};
    const auto o2 = Vector3f{1.0, 0.0, 0.0};
    const auto n2 = Vector3f{0.0, 1.0, 0.0};

    auto expected = ( p1 + p2 ) / 2.0f;

    auto actual = average_posy_vectors(
            p1, o1, n1, 1.0f,
            p2, o2, n2, 1.0f, rho );
    expect_vector_equality(expected, actual);
}

// Given two points in perpendicular planes, offset by rho/2, both claiming to be at their mesh vertex
// We expect that the negotiated coordinate will be the midpoint of the line between their respective estimates
// as projected onto pone of them
TEST_F(TestPoSy, SmoothAcrossPlanes) {
    using namespace Eigen;

    const float rho = 1.0f;
    // v1 representative vector assumes it's at origin and is
    const auto v1 = Vector3f{0.0, 0.0, 0.0};
    const auto p1 = v1 + Vector3f{0.0, 0.0, 0.0};
    const auto o1 = Vector3f{1.0, 0.0, 0.0};
    const auto n1 = Vector3f{0.0, 1.0, 0.0};

    // v2 representatiove vector assumes it's at origin and is actually at 1,0,1
    const auto v2 = Vector3f{0.5, 0.5, 0.0};
    const auto p2 = v2 + Vector3f{0.0, 0.0, 0.0};
    const auto o2 = Vector3f{0.0, 0.0, 1.0};
    const auto n2 = Vector3f{-1.0, 0.0, 0.0};

    Vector3f expected{ 0.25f, 0.0f, 0.0f};

    auto actual = average_posy_vectors(
            p1, o1, n1, 1.0f,
            p2, o2, n2, 1.0f, rho );
    expect_vector_equality(expected, actual);
}