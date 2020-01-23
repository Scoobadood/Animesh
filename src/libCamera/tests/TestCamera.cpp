#include "TestCamera.h"

#include <Camera/Camera.h>
#include <iostream>

void TestCamera::SetUp() {}

void TestCamera::TearDown() {}

Camera makeCamera() {
    Camera camera{
            (float[]) {0, 0, 10.0f},
            (float[]) {0, 0, 0},
            (float[]) {0, 1.0f, 0},
            (float[]) {640.0f, 480.0f},
            (float[]) {65.238f, 51.282f},
            5.0f};
    return camera;
}


/* ********************************************************************************
 * ** Utilities
 * ********************************************************************************/
void expect_vector_equality(Eigen::Vector3f v1, Eigen::Vector3f v2) {
    EXPECT_FLOAT_EQ(v1[0], v2[0]);
    EXPECT_FLOAT_EQ(v1[1], v2[1]);
    EXPECT_FLOAT_EQ(v1[2], v2[2]);
}

void expect_vector_equality(Eigen::Vector3f v1, float x, float y, float z) {
    EXPECT_FLOAT_EQ(v1[0], x);
    EXPECT_FLOAT_EQ(v1[1], y);
    EXPECT_FLOAT_EQ(v1[2], z);
}

/* ********************************************************************************
 * ** Test Camera construction from file
 * ********************************************************************************/
TEST_F(TestCamera, FileMissingFocalLengthShouldThrow) {
    try {
        loadCameraFromFile("camera_test_data/bad_camera_1.txt");
        FAIL() << "Expected std::invalid_argument";
    }
    catch (std::domain_error const &err) {
        EXPECT_EQ(err.what(), std::string("CAMFILE::MISSING_KEY"));
    }
    catch (...) {
        FAIL() << "Expected std::invalid_argument";
    }
}

TEST_F(TestCamera, BackprojectOriginFrom_0_0_10) {
    using namespace Eigen;

    Camera camera = makeCamera();
    Vector3f v1 = camera.to_world_coordinates(320, 240, 10.0f);

    expect_vector_equality(v1, 0.0, 0.0, 0.0f);
}

TEST_F(TestCamera, BackprojectOriginFrom_5_0_0) {
    using namespace Eigen;

    Camera camera = makeCamera();
    camera.move_to(5, 0, 0, false);
    Vector3f v1 = camera.to_world_coordinates(320, 240, 10.0f);

    expect_vector_equality(v1, -5.0, 0.0, 0.0f);
}

TEST_F(TestCamera, BackprojectOriginFrom_m5_0_0) {
    using namespace Eigen;

    Camera camera = makeCamera();
    camera.move_to(0, -5, 0, false);
    Vector3f v1 = camera.to_world_coordinates(320, 240, 10.0f);

    expect_vector_equality(v1, 0.0, 5.0, 0.0f);
}
//
// f = 5, fovx = 75, fovy = 45
TEST_F(TestCamera, BackprojectBottomLeftFrom_10_0_0) {
    using namespace Eigen;

    Camera camera = makeCamera();

// If focal point of camera is 10 units from the image plane then the corner is
// sqrt(5^2 + 2.3315384387969971^2) (latter is pixel dimension with given FOVs
// This is 5.5168896573692621

// ---------+--------
//  \       |      /
//   \      |     /
//    \     |    /
//     \    |   /
//      \   |  /
//       \ fov/
//        \ |/
//         \/
// opp/f = tan(fov/2)
// opp = f.tan(fov/2)
// Let width in wspace be 6.4 and height be 4.8
// then fov_x = 2 * atan(3.2/5) = 65.238
//      fov_y = 2 * atan(2.4/5) = 51.282
// image size in world space is 7.673269879789603 wide
//                           by 5.345111359507916

    Vector3f v1 = camera.to_world_coordinates(0, 0, 6.40225);
    float expected_depth = sqrt(-4.66 * -4.66 + 3.15 * 3.15);
    expect_vector_equality(v1,-3.2, -2.4, 5.0f);
}
