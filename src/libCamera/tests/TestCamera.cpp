#include "TestCamera.h"

#include <Camera/Camera.h>
#include <iostream>

void TestCamera::SetUp( ) {}
void TestCamera::TearDown() {}


/* ********************************************************************************
 * ** Test Camera construction from file
 * ********************************************************************************/
TEST_F( TestCamera, FileMissingFocalLengthShouldThrow ) {
  try {
    loadCameraFromFile("camera_test_data/bad_camera_1.txt");
    FAIL() << "Expected std::invalid_argument";
    }
    catch ( std::domain_error const & err ) {
        EXPECT_EQ( err.what(), std::string( "CAMFILE::MISSING_KEY") );
    }
    catch ( ... ) {
        FAIL( ) << "Expected std::invalid_argument";
    }
}

TEST_F( TestCamera, TriangleDepthReconstruction ) {
    using namespace Eigen;

    Camera camera = loadCameraFromFile("camera_test_data/tri_camera.txt");

    Matrix3f K;
    Matrix3f R;
    Vector3f t;
    decomposeCamera( camera, K, R, t );
    Vector3f v1 = backproject(275, 172, 11.06f, K, R, t );
    Vector3f v2 = backproject(367, 172, 11.06f, K, R, t );
    Vector3f v3 = backproject(321, 310, 11.05f, K, R, t );

    std::cout << "t " << t << std::endl;

    // v   -0.5            -1.0          -1.0
    // v    0.0             1.0          -1.0
    // v    0.5            -1.0          -1.0
    EXPECT_FLOAT_EQ( -0.5f, v1.x());
    EXPECT_FLOAT_EQ( -1.0f, v1.y());
    EXPECT_FLOAT_EQ( -1.0f, v1.z());

    EXPECT_FLOAT_EQ(  0.0f, v2.x());
    EXPECT_FLOAT_EQ(  1.0f, v2.y());
    EXPECT_FLOAT_EQ( -1.0f, v2.z());

    EXPECT_FLOAT_EQ(  0.5f, v3.x());
    EXPECT_FLOAT_EQ( -1.0f, v3.y());
    EXPECT_FLOAT_EQ( -1.0f, v3.z());
}

