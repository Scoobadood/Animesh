//
// Created by Dave Durbin (Old) on 7/6/21.
//

#include "TestPoSyQij.h"
#include <PoSy/PoSy.h>

void TestPoSyQij::SetUp() {}
void TestPoSyQij::TearDown() {}

const float EPSILON = 1e-4;

#define EXPECT_EQ_VECTOR3(v1,v2) \
    EXPECT_FLOAT_EQ(v1[0], v2[0]);   \
    EXPECT_FLOAT_EQ(v1[1], v2[1]);   \
    EXPECT_FLOAT_EQ(v1[2], v2[2]);


/*********************************************************************************
 **
 **                                 Test q_ij in same plane
 **
 *********************************************************************************/
TEST_F(TestPoSyQij, OnXYPlane) {
    using namespace std;
    using namespace Eigen;

    Vector3f p1{1, 2, 0};
    Vector3f p2{-2, 1, 0};
    auto expected_qij = (p1 + p2) * 0.5f;

    auto actual_qij = compute_qij(
            p1,  // Point on XY plane
            unit_z,
            p2, // Point on XY plane
            unit_z
    );
    EXPECT_EQ_VECTOR3(expected_qij, actual_qij);
}

TEST_F(TestPoSyQij, OnYZPlane) {
    using namespace std;
    using namespace Eigen;

    Vector3f p1{0, 2.5, -1};
    Vector3f p2{0, 1.5, -3};
    auto expected_qij = (p1 + p2) * 0.5f;

    auto actual_qij = compute_qij(
            p1, unit_x, // Point on YZ plane
            p2, unit_x  // Point on YZ plane
    );
    EXPECT_EQ_VECTOR3(expected_qij, actual_qij);
}

TEST_F(TestPoSyQij, OnXZPlane) {
    using namespace std;
    using namespace Eigen;

    Vector3f p1{-3, 0, -2};
    Vector3f p2{2, 0, 3};
    auto expected_qij = (p1 + p2) * 0.5f;

    auto actual_qij = compute_qij(
            p1, unit_y, p2, unit_y
    );
    EXPECT_EQ_VECTOR3(expected_qij, actual_qij);
}
/*********************************************************************************
 **
 **                                 Test q_ij in orthogonal planes
 **
 *********************************************************************************/
TEST_F(TestPoSyQij, OnIntersectionOfXZAndYZPlanes) {
    using namespace std;
    using namespace Eigen;

    Vector3f p1{2, 0, 1};
    Vector3f p2{0, 2, 1};

    Vector3f expected_qij{0, 0, 1};
    auto actual_qij = compute_qij(
            p1, unit_y,
            p2, unit_x
    );
    EXPECT_EQ_VECTOR3(expected_qij, actual_qij);
}

TEST_F(TestPoSyQij, OnIntersectionOfXZAndXYPlanes) {
    using namespace std;
    using namespace Eigen;

    Vector3f p1{2, 0, 1};
    Vector3f p2{2, 1, 0};

    Vector3f expected_qij{2, 0, 0};

    auto actual_qij = compute_qij(
            p1, unit_y,
            p2, unit_z
    );
    EXPECT_EQ_VECTOR3(expected_qij, actual_qij);
}

TEST_F(TestPoSyQij, OnIntersectionOfXYAndYZPlanes) {
    using namespace std;
    using namespace Eigen;

    Vector3f p1{2, 1.5, 0};
    Vector3f p2{0, 1.5, 3};

    Vector3f expected_qij{0, 1.5, 0};

    auto actual_qij = compute_qij(
            p1, unit_z,
            p2, unit_x
    );
    EXPECT_EQ_VECTOR3(expected_qij, actual_qij);
}
