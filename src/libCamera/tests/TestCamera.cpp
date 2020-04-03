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


TEST_F(TestCamera, depth_to_pixel_roundtrip) {
    using namespace Eigen;

    Camera camera = makeCamera();
    const auto wc = camera.to_world_coordinates(20, 25, 36.5);
    unsigned int pixelx;
    unsigned int pixely;
    float depth;
    camera.to_pixel_and_depth(wc, pixelx, pixely, depth);

    EXPECT_EQ( pixelx, 20);
    EXPECT_EQ( pixely, 25);
    EXPECT_FLOAT_EQ(depth, 36.5);
}

TEST_F(TestCamera, depth_to_pixel_roundtrip_at_multiple_resolutions) {
    using namespace Eigen;

    Camera camera = makeCamera(); // 640x480
    const auto wc = camera.to_world_coordinates(20, 26, 36.5);


    camera.set_image_size(320,240);
    unsigned int pixelx;
    unsigned int pixely;
    float depth;
    camera.to_pixel_and_depth(wc, pixelx, pixely, depth);

    EXPECT_EQ( pixelx, 10);
    EXPECT_EQ( pixely, 13);
    EXPECT_FLOAT_EQ(depth, 36.5);
}

TEST_F(TestCamera, multiple_resolutions_yield_same_depth) {
    using namespace Eigen;

    float pixel_row_depths[]{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    9.78324, 9.76742, 9.75174, 9.73618, 9.72076, 9.70547, 9.69032, 
                            9.6753, 9.66041, 9.64566, 9.63105, 9.61657, 9.60224, 9.58804,
                            9.57397, 9.56005, 9.54627, 9.53263, 9.51913, 9.50577, 9.49256, 
                            9.47948, 9.46655, 9.45377, 9.44113, 9.42864, 9.41629, 9.40409, 
                            9.39203, 9.38013, 9.36837, 9.35676, 9.3453, 9.33399, 9.32283, 
                            9.31182, 9.30096, 9.29026, 9.2797, 9.2693, 9.25906, 9.24897, 
                            9.23903, 9.22925, 9.21962, 9.21015, 9.20084, 9.19168, 9.18268, 
                            9.17385, 9.16516, 9.15664, 9.14828, 9.14007, 9.13203, 9.12415, 
                            9.11643, 9.10887, 9.10147, 9.09423, 9.08716, 9.08025, 9.0735, 
                            9.06692, 9.0605, 9.05425, 9.04816, 9.04224, 9.03648, 9.03088, 
                            9.02546, 9.0202, 9.01511, 9.01018, 9.00542, 9.00083, 8.99641,
                            8.99215, 8.98806, 8.98415, 8.98039, 8.97682, 8.9734, 8.97016,
                            8.96709, 8.96419, 8.96145, 8.95889, 8.9565, 8.95428, 8.95222,
                            8.95034, 8.94863, 8.94709, 8.94572, 8.94452, 8.9435, 8.94264,
                            8.94196, 8.94144, 8.9411, 8.94093, 8.94093, 8.9411, 8.94144,
                            8.94196, 8.94264, 8.9435, 8.94452, 8.94572, 8.94709, 8.94863,
                            8.95034, 8.95222, 8.95428, 8.9565, 8.95889, 8.96145, 8.96419,
                            8.96709, 8.97016, 8.9734, 8.97682, 8.98039, 8.98415, 8.98806,
                            8.99215, 8.99641, 9.00083, 9.00542, 9.01018, 9.01511, 9.0202,
                            9.02546, 9.03088, 9.03648, 9.04224, 9.04816, 9.05425, 9.0605,
                            9.06692, 9.0735, 9.08025, 9.08716, 9.09423, 9.10147, 9.10887,
                            9.11643, 9.12415, 9.13203, 9.14007, 9.14828, 9.15664, 9.16516,
                            9.17385, 9.18268, 9.19168, 9.20084, 9.21015, 9.21962, 9.22925,
                            9.23903, 9.24897, 9.25906, 9.2693, 9.2797, 9.29026, 9.30096,
                            9.31182, 9.32283, 9.33399, 9.3453, 9.35676, 9.36837, 9.38013,
                            9.39203, 9.40409, 9.41629, 9.42864, 9.44113, 9.45377, 9.46655,
                            9.47948, 9.49256, 9.50577, 9.51913, 9.53263, 9.54627, 9.56005,
                            9.57397, 9.58804, 9.60224, 9.61657, 9.63105, 9.64566, 9.66041,
                            9.6753, 9.69032, 9.70547, 9.72076, 9.73618, 9.75174, 9.76742, 9.78324,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };

    int num_pixels = sizeof( pixel_row_depths) / sizeof( float);
    Vector3f wc1[num_pixels];
    Camera camera = makeCamera(); // 640x480
    camera.set_image_size(320,240);
    for( int i=0; i<num_pixels; ++i ) {
        wc1[i] = camera.to_world_coordinates(i, 119, pixel_row_depths[i]);
    }

    Vector3f wc2[20];
    for( int i=0; i<20; ++i ) {
        wc2[i] = camera.to_world_coordinates(i, 7, pixel_row_depths[i*16]);
        std::cout << "320/" << (i*16) << ": (" << wc1[i*16].x() << ", " << wc1[i*16].y() << ", "<<wc1[i*16].z() << ") " <<
                " ,   20/" << i << ": (" << wc2[i].x() << ", " << wc2[i].y() << ", " << wc2[i].z()  << std::endl;
    }
}