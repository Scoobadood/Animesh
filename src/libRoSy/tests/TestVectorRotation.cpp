#include "TestVectorRotation.h"
#include "GtestUtility.h"

/*
* Test that rotating a vector around another unit vector gives the correct outptu
*/

void TestVectorRotation::SetUp(){};
void TestVectorRotation::TearDown(){};



TEST_F(TestVectorRotation, ZeroLengthOShouldThrow) { 
    int k = 0;

    EXPECT_THROW_WITH_MESSAGE(
            vector_by_rotating_around_n(zero, vec_1_0_0, k),
            std::invalid_argument,
            "Vector may not be zero length"
    );
}

TEST_F(TestVectorRotation, NonUnitNormalShouldThrow) { 
    int k = 0;
    
    try {
        vector_by_rotating_around_n(vec_1_0_0, vec_1_1_1, k);
        FAIL() << "Expected std::invalid_argument";
   }
    catch ( std::invalid_argument const & err ){
        EXPECT_EQ( err.what(), std::string( "Normal must be unit vector") );
    }
    catch( ... ) {
        FAIL( ) <<"Expected std::invalid_argument";
    }
}

    /* ********************************************************************************
     * ** Test rotations in the XY plane
     * ********************************************************************************/

TEST_F( TestVectorRotation, Rotate_2_1_0_about_0_0_1_by_0 ) {
    using namespace Eigen;

    int k = 0;

    Vector3f expected = vec_2_1_0;
    Vector3f actual = vector_by_rotating_around_n( vec_2_1_0, vec_0_0_1, k );

    EXPECT_FLOAT_EQ( expected[0], actual[0] );
    EXPECT_FLOAT_EQ( expected[1], actual[1] );
    EXPECT_FLOAT_EQ( expected[2], actual[2] );
}

TEST_F( TestVectorRotation, Rotate_2_1_0_about_0_0_1_by_1 ) {
    using namespace Eigen;

    int k = 1;

    Vector3f expected{ -1.0f, 2.0f, 0.0f};
    Vector3f actual = vector_by_rotating_around_n( vec_2_1_0, vec_0_0_1, k );

    EXPECT_FLOAT_EQ( expected[0], actual[0] );
    EXPECT_FLOAT_EQ( expected[1], actual[1] );
    EXPECT_FLOAT_EQ( expected[2], actual[2] );
}

TEST_F( TestVectorRotation, Rotate_2_1_0_about_0_0_1_by_2 ) {
    using namespace Eigen;

    int k = 2;

    Vector3f expected{ -2.0f, -1.0f, 0.0f};
    Vector3f actual = vector_by_rotating_around_n( vec_2_1_0, vec_0_0_1, k );

    EXPECT_FLOAT_EQ( expected[0], actual[0] );
    EXPECT_FLOAT_EQ( expected[1], actual[1] );
    EXPECT_FLOAT_EQ( expected[2], actual[2] );
}

TEST_F( TestVectorRotation, Rotate_2_1_0_about_0_0_1_by_3 ) {
    using namespace Eigen;

    int k = 3;

    Vector3f expected{ 1.0f, -2.0f, 0.0f};
    Vector3f actual = vector_by_rotating_around_n( vec_2_1_0, vec_0_0_1, k );

    EXPECT_FLOAT_EQ( expected[0], actual[0] );
    EXPECT_FLOAT_EQ( expected[1], actual[1] );
    EXPECT_FLOAT_EQ( expected[2], actual[2] );
}

/* ********************************************************************************
 * ** Test rotations in the XZ plane
 * ********************************************************************************/

TEST_F( TestVectorRotation, Rotate_2_0_1_about_0_1_0_by_0 ) {
    using namespace Eigen;

    int k = 0;

    Vector3f expected = vec_2_0_1;
    Vector3f actual = vector_by_rotating_around_n( vec_2_0_1, vec_0_1_0, k );

    EXPECT_FLOAT_EQ( expected[0], actual[0] );
    EXPECT_FLOAT_EQ( expected[1], actual[1] );
    EXPECT_FLOAT_EQ( expected[2], actual[2] );
}

TEST_F( TestVectorRotation, Rotate_2_0_1_about_0_1_0_by_1 ) {
    using namespace Eigen;

    int k = 1;

    Vector3f expected{ 1.0f, 0.0f, -2.0f};
    Vector3f actual = vector_by_rotating_around_n( vec_2_0_1, vec_0_1_0, k );

    EXPECT_FLOAT_EQ( expected[0], actual[0] );
    EXPECT_FLOAT_EQ( expected[1], actual[1] );
    EXPECT_FLOAT_EQ( expected[2], actual[2] );
}

TEST_F( TestVectorRotation, Rotate_2_0_1_about_0_1_0_by_2 ) {
    using namespace Eigen;

    int k = 2;

    Vector3f expected{ -2.0f, 0.0f, -1.0f};
    Vector3f actual = vector_by_rotating_around_n( vec_2_0_1, vec_0_1_0, k );

    EXPECT_FLOAT_EQ( expected[0], actual[0] );
    EXPECT_FLOAT_EQ( expected[1], actual[1] );
    EXPECT_FLOAT_EQ( expected[2], actual[2] );
}

TEST_F( TestVectorRotation, Rotate_2_0_1_about_0_1_0_by_3 ) {
    using namespace Eigen;

    int k = 3;

    Vector3f expected{ -1.0f, 0.0f, 2.0f};
    Vector3f actual = vector_by_rotating_around_n( vec_2_0_1, vec_0_1_0, k );

    EXPECT_FLOAT_EQ( expected[0], actual[0] );
    EXPECT_FLOAT_EQ( expected[1], actual[1] );
    EXPECT_FLOAT_EQ( expected[2], actual[2] );
}

/* ********************************************************************************
 * ** Test rotations in the YZ plane
 * ********************************************************************************/
TEST_F( TestVectorRotation, Rotate_0_2_1_about_1_0_0_by_0 ) {
    using namespace Eigen;

    int k = 0;

    Vector3f expected = vec_0_2_1;
    Vector3f actual = vector_by_rotating_around_n( vec_0_2_1, vec_1_0_0, k );

    EXPECT_FLOAT_EQ( expected[0], actual[0] );
    EXPECT_FLOAT_EQ( expected[1], actual[1] );
    EXPECT_FLOAT_EQ( expected[2], actual[2] );
}

TEST_F( TestVectorRotation, Rotate_0_1_2_about_1_0_0_by_1 ) {
    using namespace Eigen;

    int k = 1;

    Vector3f expected{ 0.0f, -1.0f, 2.0f};
    Vector3f actual = vector_by_rotating_around_n( vec_0_2_1, vec_1_0_0, k );

    EXPECT_FLOAT_EQ( expected[0], actual[0] );
    EXPECT_FLOAT_EQ( expected[1], actual[1] );
    EXPECT_FLOAT_EQ( expected[2], actual[2] );
}

TEST_F( TestVectorRotation, Rotate_0_1_2_about_1_0_0_by_2 ) {
    using namespace Eigen;

    int k = 2;

    Vector3f expected{ 0.0f, -2.0f, -1.0f};
    Vector3f actual = vector_by_rotating_around_n( vec_0_2_1, vec_1_0_0, k );

    EXPECT_FLOAT_EQ( expected[0], actual[0] );
    EXPECT_FLOAT_EQ( expected[1], actual[1] );
    EXPECT_FLOAT_EQ( expected[2], actual[2] );
}

TEST_F( TestVectorRotation, Rotate_0_1_2_about_1_0_0_by_3 ) {
    using namespace Eigen;

    int k = 3;

    Vector3f expected{ 0.0f, 1.0f, -2.0f};
    Vector3f actual = vector_by_rotating_around_n( vec_0_2_1, vec_1_0_0, k );

    EXPECT_FLOAT_EQ( expected[0], actual[0] );
    EXPECT_FLOAT_EQ( expected[1], actual[1] );
    EXPECT_FLOAT_EQ( expected[2], actual[2] );
}