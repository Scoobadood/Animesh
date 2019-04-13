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
    loadCameraFromFile("data/bad_camera_1.txt");
    FAIL() << "Expected std::invalid_argument";
    }
    catch ( std::invalid_argument const & err ) {
        EXPECT_EQ( err.what(), std::string( "Missing focal length in camera file") );
    }
    catch ( ... ) {
        FAIL( ) << "Expected std::invalid_argument";
    }
}

TEST_F( TestCamera, TriangleDepthReconstruction ) {
    using namespace Eigen;

    Camera camera = loadCameraFromFile("data/tri_camera.txt");

    Matrix3f K;
    Matrix3f R;
    Vector3f t;
    decomposeCamera( camera, K, R, t );
    Vector3f v1 = backproject(275, 172, 11.06f, K, R, t );
    Vector3f v2 = backproject(367, 172, 11.06f, K, R, t );
    Vector3f v3 = backproject(321, 310, 11.05f, K, R, t );
}

