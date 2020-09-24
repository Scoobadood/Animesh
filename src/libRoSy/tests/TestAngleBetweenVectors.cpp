//#include "TestAngleBetweenVectors.h"
#include "gtest/gtest.h"
#include <RoSy/RoSy.h>
#include "GtestUtility.h"

namespace {
TEST(TestAngleBetweenVectors, ZeroLengthVectorsShouldThrow) {
        Eigen::Vector3f v1{0.1f, 0.2f, 0.3f};
        Eigen::Vector3f v2{0.0f, 0.0f, 0.0f};

        EXPECT_THROW_WITH_MESSAGE(
                degrees_angle_between_vectors(v1, v2),
                std::invalid_argument,
                "Vector may not be zero length"
        );
    }


TEST(TestAngleBetweenVectors, IdenticalVectorsShouldReturnZero) {
    Eigen::Vector3f v1{ 0.1f, 0.2f, 0.3f };
    Eigen::Vector3f v2{ 0.1f, 0.2f, 0.3f };

    float expected = 0.0f;
    float actual   = degrees_angle_between_vectors(v1, v2);

    EXPECT_FLOAT_EQ( expected, actual );
}

//
// Z Plane
//

// 45 Degrees
TEST(TestAngleBetweenVectors, Test45DegreesShouldReturn_45) {
    Eigen::Vector3f v1{ 2.0f, 0.0f, 0.0f };
    Eigen::Vector3f v2{ 1.0f, 1.0f, 0.0f };

    float expected = 45.0f;
    float actual   = degrees_angle_between_vectors(v1, v2);

    EXPECT_FLOAT_EQ( expected, actual );
}

// 90 Degrees
TEST(TestAngleBetweenVectors, Test90DegreesShouldReturn_90) {
    Eigen::Vector3f v1{ 2.0f, 0.0f, 0.0f };
    Eigen::Vector3f v2{ 0.0f, 3.0f, 0.0f };

    float expected = 90.0f;
    float actual   = degrees_angle_between_vectors(v1, v2);

    EXPECT_FLOAT_EQ( expected, actual );
}

// 135 Degrees
TEST(TestAngleBetweenVectors, Test135DegreesShouldReturn_135) {
    Eigen::Vector3f v1{ 2.0f, 0.0f, 0.0f };
    Eigen::Vector3f v2{ -1.0f, 1.0f, 0.0f };

    float expected = 135.0f;
    float actual   = degrees_angle_between_vectors(v1, v2);

    EXPECT_FLOAT_EQ( expected, actual );
}

// 180 Degrees
TEST(TestAngleBetweenVectors, Test180DegreesShouldReturn_180) {
    Eigen::Vector3f v1{ 2.0f, 0.0f, 0.0f };
    Eigen::Vector3f v2{ -2.0f, 0.0f, 0.0f };

    float expected = 180.0f;
    float actual   = degrees_angle_between_vectors(v1, v2);

    EXPECT_FLOAT_EQ( expected, actual );
}


// 225 Degrees
TEST(TestAngleBetweenVectors, Test225DegreesShouldReturn_135) {
    Eigen::Vector3f v1{ 2.0f, 0.0f, 0.0f };
    Eigen::Vector3f v2{ -1.0f, -1.0f, 0.0f };

    float expected = 135.0f;
    float actual   = degrees_angle_between_vectors(v1, v2);

    EXPECT_FLOAT_EQ( expected, actual );
}

// 270 Degrees
TEST(TestAngleBetweenVectors, Test270DegreesShouldReturn_90) {
    Eigen::Vector3f v1{ 2.0f, 0.0f, 0.0f };
    Eigen::Vector3f v2{ 0.0f, -1.0f, 0.0f };

    float expected = 90.0f;
    float actual   = degrees_angle_between_vectors(v1, v2);

    EXPECT_FLOAT_EQ( expected, actual );
}

// 315 Degrees
TEST(TestAngleBetweenVectors, Test315DegreesShouldReturn_45) {
    Eigen::Vector3f v1{ 2.0f, 0.0f, 0.0f };
    Eigen::Vector3f v2{ 1.0f, -1.0f, 0.0f };

    float expected = 45.0f;
    float actual   = degrees_angle_between_vectors(v1, v2);

    EXPECT_FLOAT_EQ( expected, actual );
}

// 360 Degrees
TEST(TestAngleBetweenVectors, Test360DegreesShouldReturnZero) {
    Eigen::Vector3f v1{ 2.0f, 0.0f, 0.0f };
    Eigen::Vector3f v2{ 2.0f, 0.0f, 0.0f };

    float expected = 0.0f;
    float actual   = degrees_angle_between_vectors(v1, v2);

    EXPECT_FLOAT_EQ( expected, actual );
}


//
// XZ Plane
//

// 45 Degrees
TEST(TestAngleBetweenVectors, Test45DegreesInYPlaneShouldReturn_45) {
    Eigen::Vector3f v1{ 2.0f, 0.0f, 0.0f };
    Eigen::Vector3f v2{ 1.0f, 0.0f, 1.0f };

    float expected = 45.0f;
    float actual   = degrees_angle_between_vectors(v1, v2);

    EXPECT_FLOAT_EQ( expected, actual );
}

// 90 Degrees
TEST(TestAngleBetweenVectors, Test90DegreesInYPlaneShouldReturn_90) {
    Eigen::Vector3f v1{ 2.0f, 0.0f, 0.0f };
    Eigen::Vector3f v2{ 0.0f, 0.0f, 3.0f };

    float expected = 90.0f;
    float actual   = degrees_angle_between_vectors(v1, v2);

    EXPECT_FLOAT_EQ( expected, actual );
}

// 135 Degrees
TEST(TestAngleBetweenVectors, Test135DegreesInYPlaneShouldReturn_135) {
    Eigen::Vector3f v1{ 2.0f, 0.0f, 0.0f };
    Eigen::Vector3f v2{ -1.0f, 0.0f, 1.0f };

    float expected = 135.0f;
    float actual   = degrees_angle_between_vectors(v1, v2);

    EXPECT_FLOAT_EQ( expected, actual );
}


// 225 Degrees
TEST(TestAngleBetweenVectors, Test225DegreesInYPlaneShouldReturn_135) {
    Eigen::Vector3f v1{ 2.0f, 0.0f, 0.0f };
    Eigen::Vector3f v2{ -1.0f, 0.0f, -1.0f };

    float expected = 135.0f;
    float actual   = degrees_angle_between_vectors(v1, v2);

    EXPECT_FLOAT_EQ( expected, actual );
}

// 270 Degrees
TEST(TestAngleBetweenVectors, Test270DegreesInYPlaneShouldReturn_90) {
    Eigen::Vector3f v1{ 2.0f, 0.0f, 0.0f };
    Eigen::Vector3f v2{ 0.0f, 0.0f, -1.0f };

    float expected = 90.0f;
    float actual   = degrees_angle_between_vectors(v1, v2);

    EXPECT_FLOAT_EQ( expected, actual );
}

// 315 Degrees
TEST(TestAngleBetweenVectors, Test315DegreesInYPlaneShouldReturn_45) {
    Eigen::Vector3f v1{ 2.0f, 0.0f, 0.0f };
    Eigen::Vector3f v2{ 1.0f, 0.0f, -1.0f };

    float expected = 45.0f;
    float actual   = degrees_angle_between_vectors(v1, v2);

    EXPECT_FLOAT_EQ( expected, actual );
}


//
// YZ Plane
//

// 45 Degrees
TEST(TestAngleBetweenVectors, Test45DegreesInXPlaneShouldReturn_45) {
    Eigen::Vector3f v1{ 0.0f, 0.0f, 2.0f };
    Eigen::Vector3f v2{ 0.0f, 1.0f, 1.0f };

    float expected = 45.0f;
    float actual   = degrees_angle_between_vectors(v1, v2);

    EXPECT_FLOAT_EQ( expected, actual );
}

// 135 Degrees
TEST(TestAngleBetweenVectors, Test135DegreesInXPlaneShouldReturn_135) {
    Eigen::Vector3f v1{ 0.0f, 0.0f, 2.0f };
    Eigen::Vector3f v2{ 0.0f, 1.0f, -1.0f };

    float expected = 135.0f;
    float actual   = degrees_angle_between_vectors(v1, v2);

    EXPECT_FLOAT_EQ( expected, actual );
}

// 180 Degrees
TEST(TestAngleBetweenVectors, Test180DegreesInXPlaneShouldReturn_180) {
    Eigen::Vector3f v1{ 0.0f, 0.0f, 2.0f };
    Eigen::Vector3f v2{ 0.0f, 0.0f, -2.0f };

    float expected = 180.0f;
    float actual   = degrees_angle_between_vectors(v1, v2);

    EXPECT_FLOAT_EQ( expected, actual );
}


// 225 Degrees
TEST(TestAngleBetweenVectors, Test225DegreesInXPlaneShouldReturn_135) {
    Eigen::Vector3f v1{ 0.0f, 0.0f, 2.0f };
    Eigen::Vector3f v2{ 0.0f, -1.0f, -1.0f };

    float expected = 135.0f;
    float actual   = degrees_angle_between_vectors(v1, v2);

    EXPECT_FLOAT_EQ( expected, actual );
}


// 315 Degrees
TEST(TestAngleBetweenVectors, Test315DegreesInXPlaneShouldReturn_45) {
    Eigen::Vector3f v1{ 0.0f, 0.0f, 2.0f };
    Eigen::Vector3f v2{ 0.0f, -1.0f, 1.0f };

    float expected = 45.0f;
    float actual   = degrees_angle_between_vectors(v1, v2);

    EXPECT_FLOAT_EQ( expected, actual );
}

// 360 Degrees
TEST(TestAngleBetweenVectors, Test360DegreesInXPlaneShouldReturnZero) {
    Eigen::Vector3f v1{ 2.0f, 0.0f, 0.0f };
    Eigen::Vector3f v2{ 2.0f, 0.0f, 0.0f };

    float expected = 0.0f;
    float actual   = degrees_angle_between_vectors(v1, v2);

    EXPECT_FLOAT_EQ( expected, actual );
}
}





