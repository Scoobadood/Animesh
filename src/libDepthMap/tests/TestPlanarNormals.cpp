#include "TestPlanarNormals.h"

#include <iostream>
#include <cmath>
const float INV_SQRT_2 = 1.0f / std::sqrt(2.0f);

void TestPlanarNormals::SetUp( ) {}
void TestPlanarNormals::TearDown() {}

Camera get_camera( ) {
    float pos[]{0.0, 100.0, 100.0};
    float view[]{0.0, 0.0, 0.0};
    float up[]{0.0, 1.0, 0.0};
    float res[]{640, 480};
    float fov[]{75.0, 56.25};
    Camera camera{
            pos, view, up, res, fov, 5.0
    };
    return camera;
}

/* ********************************************************************************
 * ** Test DepthMap construction from file
 * ********************************************************************************/
TEST_F( TestPlanarNormals,  aTest ) {
EXPECT_TRUE( true);
}
