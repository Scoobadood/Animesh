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