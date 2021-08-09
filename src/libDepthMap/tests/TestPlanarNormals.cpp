#include "TestPlanarNormals.h"

#include <DepthMap/PlaneFittingNormals.h>
#include <iostream>
#include <cmath>

#include "gtest/gtest.h"


void TestPlanarNormals::SetUp( ) {}
void TestPlanarNormals::TearDown() {}

Camera get_camera_z( ) {
    float pos[]{0.0, 0.0, -20.0};
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
TEST_F( TestPlanarNormals,  NormalsShouldBeCorrect ) {
    using namespace std;

    float data[] {
            0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
            0.0, 3.0, 3.0, 3.0, 3.0, 3.0, 0.0,
            0.0, 3.0, 3.0, 3.0, 3.0, 3.0, 0.0,
            0.0, 3.0, 3.0, 3.0, 3.0, 3.0, 0.0,
            0.0, 3.0, 3.0, 3.0, 3.0, 3.0, 0.0,
            0.0, 3.0, 3.0, 3.0, 3.0, 3.0, 0.0,
            0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0
    };
    // 7x7 map with 5x5 set in centre.
    DepthMap d{7, 7, data};
    vector<vector<NormalWithType>> normals = compute_normals_from_neighbours(&d, get_camera_z());

    EXPECT_EQ(normals.size(), 7);
}

