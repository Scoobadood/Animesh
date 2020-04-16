//#include "TestAngleBetweenVectors.h"
#include "gtest/gtest.h"
#include <RoSy/RoSy.h>

namespace {
TEST(TestAngleBetweenVectors, ZeroLengthVectorsShouldThrow) {
    Eigen::Vector3f v1{ 0.1f, 0.2f, 0.3f };
    Eigen::Vector3f v2{ 0.0f, 0.0f, 0.0f };

    try {
        degrees_angle_between_vectors(v1, v2);
        FAIL() << "Expected std::invalid_argument";
    }
    catch ( std::invalid_argument const & err ) {
        EXPECT_EQ( err.what(), std::string( "Vector may not be zero length") );
    }
    catch ( ... ) {
        FAIL( ) << "Expected std::invalid_argument";
    }
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
TEST(TestAngleBetweenVectors, Test45DegreesShouldReturnPIOverFour) {
    Eigen::Vector3f v1{ 2.0f, 0.0f, 0.0f };
    Eigen::Vector3f v2{ 1.0f, 1.0f, 0.0f };

    float expected = M_PI / 4.0f;
    float actual   = degrees_angle_between_vectors(v1, v2);

    EXPECT_FLOAT_EQ( expected, actual );
}

// 90 Degrees
TEST(TestAngleBetweenVectors, Test90DegreesShouldReturnPIOverTwo) {
    Eigen::Vector3f v1{ 2.0f, 0.0f, 0.0f };
    Eigen::Vector3f v2{ 0.0f, 3.0f, 0.0f };

    float expected = M_PI / 2.0f;
    float actual   = degrees_angle_between_vectors(v1, v2);

    EXPECT_FLOAT_EQ( expected, actual );
}

// 135 Degrees
TEST(TestAngleBetweenVectors, Test135DegreesShouldReturnThreePIOverFour) {
    Eigen::Vector3f v1{ 2.0f, 0.0f, 0.0f };
    Eigen::Vector3f v2{ -1.0f, 1.0f, 0.0f };

    float expected = 3 * M_PI / 4.0f;
    float actual   = degrees_angle_between_vectors(v1, v2);

    EXPECT_FLOAT_EQ( expected, actual );
}

// 180 Degrees
TEST(TestAngleBetweenVectors, Test180DegreesShouldReturnPI) {
    Eigen::Vector3f v1{ 2.0f, 0.0f, 0.0f };
    Eigen::Vector3f v2{ -2.0f, 0.0f, 0.0f };

    float expected = M_PI;
    float actual   = degrees_angle_between_vectors(v1, v2);

    EXPECT_FLOAT_EQ( expected, actual );
}


// 225 Degrees
TEST(TestAngleBetweenVectors, Test225DegreesShouldReturnFivePIOverFour) {
    Eigen::Vector3f v1{ 2.0f, 0.0f, 0.0f };
    Eigen::Vector3f v2{ -1.0f, -1.0f, 0.0f };

    float expected = 5 * M_PI / 4.0f;
    float actual   = degrees_angle_between_vectors(v1, v2);

    EXPECT_FLOAT_EQ( expected, actual );
}

// 270 Degrees
TEST(TestAngleBetweenVectors, Test270DegreesShouldReturnThreePIOverTwo) {
    Eigen::Vector3f v1{ 2.0f, 0.0f, 0.0f };
    Eigen::Vector3f v2{ 0.0f, -1.0f, 0.0f };

    float expected = 3 * M_PI / 2.0f;
    float actual   = degrees_angle_between_vectors(v1, v2);

    EXPECT_FLOAT_EQ( expected, actual );
}

// 315 Degrees
TEST(TestAngleBetweenVectors, Test315DegreesShouldReturnSevenPIOverFour) {
    Eigen::Vector3f v1{ 2.0f, 0.0f, 0.0f };
    Eigen::Vector3f v2{ 1.0f, -1.0f, 0.0f };

    float expected = 7 * M_PI / 4.0f;
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
TEST(TestAngleBetweenVectors, Test45DegreesInYPlaneShouldReturnPIOverFour) {
    Eigen::Vector3f v1{ 2.0f, 0.0f, 0.0f };
    Eigen::Vector3f v2{ 1.0f, 0.0f, 1.0f };

    float expected = M_PI / 4.0f;
    float actual   = degrees_angle_between_vectors(v1, v2);

    EXPECT_FLOAT_EQ( expected, actual );
}

// 90 Degrees
TEST(TestAngleBetweenVectors, Test90DegreesInYPlaneShouldReturnPIOverTwo) {
    Eigen::Vector3f v1{ 2.0f, 0.0f, 0.0f };
    Eigen::Vector3f v2{ 0.0f, 0.0f, 3.0f };

    float expected = M_PI / 2.0f;
    float actual   = degrees_angle_between_vectors(v1, v2);

    EXPECT_FLOAT_EQ( expected, actual );
}

// 135 Degrees
TEST(TestAngleBetweenVectors, Test135DegreesInYPlaneShouldReturnThreePIOverFour) {
    Eigen::Vector3f v1{ 2.0f, 0.0f, 0.0f };
    Eigen::Vector3f v2{ -1.0f, 0.0f, 1.0f };

    float expected = 3 * M_PI / 4.0f;
    float actual   = degrees_angle_between_vectors(v1, v2);

    EXPECT_FLOAT_EQ( expected, actual );
}


// 225 Degrees
TEST(TestAngleBetweenVectors, Test225DegreesInYPlaneShouldReturnFivePIOverFour) {
    Eigen::Vector3f v1{ 2.0f, 0.0f, 0.0f };
    Eigen::Vector3f v2{ -1.0f, 0.0f, -1.0f };

    float expected = 5 * M_PI / 4.0f;
    float actual   = degrees_angle_between_vectors(v1, v2);

    EXPECT_FLOAT_EQ( expected, actual );
}

// 270 Degrees
TEST(TestAngleBetweenVectors, Test270DegreesInYPlaneShouldReturnThreePIOverTwo) {
    Eigen::Vector3f v1{ 2.0f, 0.0f, 0.0f };
    Eigen::Vector3f v2{ 0.0f, 0.0f, -1.0f };

    float expected = 3 * M_PI / 2.0f;
    float actual   = degrees_angle_between_vectors(v1, v2);

    EXPECT_FLOAT_EQ( expected, actual );
}

// 315 Degrees
TEST(TestAngleBetweenVectors, Test315DegreesInYPlaneShouldReturnSevenPIOverFour) {
    Eigen::Vector3f v1{ 2.0f, 0.0f, 0.0f };
    Eigen::Vector3f v2{ 1.0f, 0.0f, -1.0f };

    float expected = 7 * M_PI / 4.0f;
    float actual   = degrees_angle_between_vectors(v1, v2);

    EXPECT_FLOAT_EQ( expected, actual );
}


//
// YZ Plane
//

// 45 Degrees
TEST(TestAngleBetweenVectors, Test45DegreesInXPlaneShouldReturnPIOverFour) {
    Eigen::Vector3f v1{ 0.0f, 0.0f, 2.0f };
    Eigen::Vector3f v2{ 0.0f, 1.0f, 1.0f };

    float expected = M_PI / 4.0f;
    float actual   = degrees_angle_between_vectors(v1, v2);

    EXPECT_FLOAT_EQ( expected, actual );
}

// 135 Degrees
TEST(TestAngleBetweenVectors, Test135DegreesInXPlaneShouldReturnThreePIOverFour) {
    Eigen::Vector3f v1{ 0.0f, 0.0f, 2.0f };
    Eigen::Vector3f v2{ 0.0f, 1.0f, -1.0f };

    float expected = 3 * M_PI / 4.0f;
    float actual   = degrees_angle_between_vectors(v1, v2);

    EXPECT_FLOAT_EQ( expected, actual );
}

// 180 Degrees
TEST(TestAngleBetweenVectors, Test180DegreesInXPlaneShouldReturnPI) {
    Eigen::Vector3f v1{ 0.0f, 0.0f, 2.0f };
    Eigen::Vector3f v2{ 0.0f, 0.0f, -2.0f };

    float expected = M_PI;
    float actual   = degrees_angle_between_vectors(v1, v2);

    EXPECT_FLOAT_EQ( expected, actual );
}


// 225 Degrees
TEST(TestAngleBetweenVectors, Test225DegreesInXPlaneShouldReturnFivePIOverFour) {
    Eigen::Vector3f v1{ 0.0f, 0.0f, 2.0f };
    Eigen::Vector3f v2{ 0.0f, -1.0f, -1.0f };

    float expected = 5 * M_PI / 4.0f;
    float actual   = degrees_angle_between_vectors(v1, v2);

    EXPECT_FLOAT_EQ( expected, actual );
}


// 315 Degrees
TEST(TestAngleBetweenVectors, Test315DegreesInXPlaneShouldReturnSevenPIOverFour) {
    Eigen::Vector3f v1{ 0.0f, 0.0f, 2.0f };
    Eigen::Vector3f v2{ 0.0f, -1.0f, 1.0f };

    float expected = 7 * M_PI / 4.0f;
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





