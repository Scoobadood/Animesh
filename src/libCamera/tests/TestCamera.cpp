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
    Vector3f v1 = backproject(camera, 275, 172, 11.06f);
    Vector3f v2 = backproject(camera, 321, 310, 11.05f);
    Vector3f v3 = backproject(camera, 367, 172, 11.06f);

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

