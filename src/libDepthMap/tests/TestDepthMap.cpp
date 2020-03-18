#include "TestDepthMap.h"

#include <iostream>
#include <cmath>
const float INV_SQRT_2 = 1.0f / std::sqrt(2.0f);

void TestDepthMap::SetUp( ) {}
void TestDepthMap::TearDown() {}

unsigned int TestDepthMap::count_non_zero_cells(DepthMap& d) {
	unsigned int count = 0;
	for( int r = 0; r < d.height(); ++r) {
		for( int c = 0; c < d.width(); ++c ) {
			if( d.depth_at(c,r) > 0.0f ) {
				count++;
			}
		}
	}
	return count;
}

/* ********************************************************************************
 * ** Helpers
 * ********************************************************************************/
void dump(const DepthMap& d ) {
	// Dump it for interest
	for( int r = 0; r < d.height(); r++ ) {
		for( int c = 0; c < d.width(); c++ ) {
			std::cout << d.depth_at(c,r) << "  ";
		}
		std::cout << std::endl;
	}
}

bool flag_is_set(unsigned int all_flags, DepthMap::tDirection flag ) {
    return( (all_flags & flag) == flag );
}

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
TEST_F( TestDepthMap, ShouldParseDimensions ) {
	DepthMap d{"depthmap_test_data/dm.dat"};
	EXPECT_EQ( d.width(), 640);
	EXPECT_EQ( d.height(), 480);
}

class TestDM : DepthMap {
private:
    bool connect8;

public:
    TestDM( unsigned int w, unsigned int h, bool connect8) : DepthMap(w,h,nullptr){
        this->connect8 = connect8;
    }

    void testFlags(unsigned int x, unsigned int y, bool expected, tDirection flag) {
        unsigned int flags = get_valid_directions(x, y, connect8);
        if( expected) {
            EXPECT_EQ(flag, flags & flag);
        } else {
            EXPECT_EQ(0, flags & flag);
        }
    }
};

TEST_F( TestDepthMap, AllFlagsShouldBeSetForCentre ) {
    TestDM d{4, 4, true};
    d.testFlags(1, 1, true, DepthMap::UP);
    d.testFlags(1, 1, true, DepthMap::UP_LEFT);
    d.testFlags(1, 1, true, DepthMap::UP_RIGHT);
    d.testFlags(1, 1, true, DepthMap::LEFT);
    d.testFlags(1, 1, true, DepthMap::RIGHT);
    d.testFlags(1, 1, true, DepthMap::DOWN);
    d.testFlags(1, 1, true, DepthMap::DOWN_LEFT);
    d.testFlags(1, 1, true, DepthMap::DOWN_RIGHT);
}

TEST_F( TestDepthMap, FourFlagsShouldBeSetForCentreWithFour ) {
    TestDM d{4, 4, false};
    d.testFlags(1, 1, true, DepthMap::UP);
    d.testFlags(1, 1, false, DepthMap::UP_LEFT);
    d.testFlags(1, 1, false, DepthMap::UP_RIGHT);
    d.testFlags(1, 1, true, DepthMap::LEFT);
    d.testFlags(1, 1, true, DepthMap::RIGHT);
    d.testFlags(1, 1, true, DepthMap::DOWN);
    d.testFlags(1, 1, false, DepthMap::DOWN_LEFT);
    d.testFlags(1, 1, false, DepthMap::DOWN_RIGHT);
}

TEST_F( TestDepthMap, ThreeFlagsShouldBeSetForTopLeftCorner ) {
    TestDM d{4, 4, true};
    d.testFlags(0, 0, false, DepthMap::UP);
    d.testFlags(0, 0, false, DepthMap::UP_LEFT);
    d.testFlags(0, 0, false, DepthMap::UP_RIGHT);
    d.testFlags(0, 0, false, DepthMap::LEFT);
    d.testFlags(0, 0, true, DepthMap::RIGHT);
    d.testFlags(0, 0, true, DepthMap::DOWN);
    d.testFlags(0, 0, false, DepthMap::DOWN_LEFT);
    d.testFlags(0, 0, true, DepthMap::DOWN_RIGHT);
}

TEST_F( TestDepthMap, ThreeFlagsShouldBeSetForTopRightCorner ) {
    TestDM d{4, 4, true};
    d.testFlags(3, 0, false, DepthMap::UP);
    d.testFlags(3, 0, false, DepthMap::UP_LEFT);
    d.testFlags(3, 0, false, DepthMap::UP_RIGHT);
    d.testFlags(3, 0, true, DepthMap::LEFT);
    d.testFlags(3, 0, false, DepthMap::RIGHT);
    d.testFlags(3, 0, true, DepthMap::DOWN);
    d.testFlags(3, 0, true, DepthMap::DOWN_LEFT);
    d.testFlags(3, 0, false, DepthMap::DOWN_RIGHT);
}

TEST_F( TestDepthMap, ThreeFlagsShouldBeSetForBottomLeftCorner ) {
    TestDM d{4, 4, true};
    d.testFlags(0, 3, true, DepthMap::UP);
    d.testFlags(0, 3, false, DepthMap::UP_LEFT);
    d.testFlags(0, 3, true, DepthMap::UP_RIGHT);
    d.testFlags(0, 3, false, DepthMap::LEFT);
    d.testFlags(0, 3, true, DepthMap::RIGHT);
    d.testFlags(0, 3, false, DepthMap::DOWN);
    d.testFlags(0, 3, false, DepthMap::DOWN_LEFT);
    d.testFlags(0, 3, false, DepthMap::DOWN_RIGHT);
}

TEST_F( TestDepthMap, ThreeFlagsShouldBeSetForBottomRightCorner ) {
    TestDM d{4, 4, true};
    d.testFlags(3, 3, true, DepthMap::UP);
    d.testFlags(3, 3, true, DepthMap::UP_LEFT);
    d.testFlags(3, 3, false, DepthMap::UP_RIGHT);
    d.testFlags(3, 3, true, DepthMap::LEFT);
    d.testFlags(3, 3, false, DepthMap::RIGHT);
    d.testFlags(3, 3, false, DepthMap::DOWN);
    d.testFlags(3, 3, false, DepthMap::DOWN_LEFT);
    d.testFlags(3, 3, false, DepthMap::DOWN_RIGHT);
}

TEST_F( TestDepthMap, TwoFlagsShouldBeSetForTopLeftCornerWithFour ) {
    TestDM d{4, 4, false};
    d.testFlags(0, 0, false, DepthMap::UP);
    d.testFlags(0, 0, false, DepthMap::UP_LEFT);
    d.testFlags(0, 0, false, DepthMap::UP_RIGHT);
    d.testFlags(0, 0, false, DepthMap::LEFT);
    d.testFlags(0, 0, true, DepthMap::RIGHT);
    d.testFlags(0, 0, true, DepthMap::DOWN);
    d.testFlags(0, 0, false, DepthMap::DOWN_LEFT);
    d.testFlags(0, 0, false, DepthMap::DOWN_RIGHT);
}

TEST_F( TestDepthMap, TwoFlagsShouldBeSetForTopRightCornerWithFour ) {
    TestDM d{4, 4, false};
    d.testFlags(3, 0, false, DepthMap::UP);
    d.testFlags(3, 0, false, DepthMap::UP_LEFT);
    d.testFlags(3, 0, false, DepthMap::UP_RIGHT);
    d.testFlags(3, 0, true, DepthMap::LEFT);
    d.testFlags(3, 0, false, DepthMap::RIGHT);
    d.testFlags(3, 0, true, DepthMap::DOWN);
    d.testFlags(3, 0, false, DepthMap::DOWN_LEFT);
    d.testFlags(3, 0, false, DepthMap::DOWN_RIGHT);
}

TEST_F( TestDepthMap, TwoFlagsShouldBeSetForBottomLeftCornerWithFour ) {
    TestDM d{4, 4, false};
    d.testFlags(0, 3, true, DepthMap::UP);
    d.testFlags(0, 3, false, DepthMap::UP_LEFT);
    d.testFlags(0, 3, false, DepthMap::UP_RIGHT);
    d.testFlags(0, 3, false, DepthMap::LEFT);
    d.testFlags(0, 3, true, DepthMap::RIGHT);
    d.testFlags(0, 3, false, DepthMap::DOWN);
    d.testFlags(0, 3, false, DepthMap::DOWN_LEFT);
    d.testFlags(0, 3, false, DepthMap::DOWN_RIGHT);
}

TEST_F( TestDepthMap, TwoFlagsShouldBeSetForBottomRightCornerWithFour ) {
    TestDM d{4, 4, false};
    d.testFlags(3, 3, true, DepthMap::UP);
    d.testFlags(3, 3, false, DepthMap::UP_LEFT);
    d.testFlags(3, 3, false, DepthMap::UP_RIGHT);
    d.testFlags(3, 3, true, DepthMap::LEFT);
    d.testFlags(3, 3, false, DepthMap::RIGHT);
    d.testFlags(3, 3, false, DepthMap::DOWN);
    d.testFlags(3, 3, false, DepthMap::DOWN_LEFT);
    d.testFlags(3, 3, false, DepthMap::DOWN_RIGHT);
}

TEST_F( TestDepthMap, HorizontalEdgeTest ) {
	DepthMap d{"depthmap_test_data/horizontal_edge_test.dat"};
	unsigned int count = count_non_zero_cells(d);
	EXPECT_EQ( count, 25);

	d.cull_unreliable_depths(1.9f, 0.5f);

	// Count how many cells have non-0 depths.
	count = count_non_zero_cells(d);

	EXPECT_EQ( count, 22);
}

TEST_F( TestDepthMap, VerticalEdgeTest ) {
	DepthMap d{"depthmap_test_data/vertical_edge_test.dat"};
	unsigned int count = count_non_zero_cells(d);
	EXPECT_EQ( count, 25);

	d.cull_unreliable_depths(1.9f, 0.5f);

	// Count how many cells have non-0 depths.
	count = count_non_zero_cells(d);

	EXPECT_EQ( count, 22);
}

TEST_F( TestDepthMap, SolidBlockTest ) {
	DepthMap d{"depthmap_test_data/solid_block_test.dat"};
	unsigned int count = count_non_zero_cells(d);
	EXPECT_EQ( count, 25);

	d.cull_unreliable_depths(2.0f, 0.5f);

	// Count how many cells have non-0 depths.
	count = count_non_zero_cells(d);

	EXPECT_EQ( count, 25);
}

TEST_F( TestDepthMap, StepEdgeTest ) {
	DepthMap d{"depthmap_test_data/step_test.dat"};
	unsigned int count = count_non_zero_cells(d);
	EXPECT_EQ( count, 25);

	d.cull_unreliable_depths(2.0f, 0.5f);

	// Count how many cells have non-0 depths.
	count = count_non_zero_cells(d);

	EXPECT_EQ( count, 25);
}

TEST_F( TestDepthMap, NoisyCentreTest ) {
	DepthMap d{"depthmap_test_data/noisy_centre_test.dat"};
	unsigned int count = count_non_zero_cells(d);
	EXPECT_EQ( count, 25);

	d.cull_unreliable_depths(2.0f, 0.5f);

	// Count how many cells have non-0 depths.
	count = count_non_zero_cells(d);

	EXPECT_EQ( count, 24);
}

TEST_F( TestDepthMap, DiagonalEdgeTest ) {
	DepthMap d{"depthmap_test_data/diagonal_edge_test.dat"};
	unsigned int count = count_non_zero_cells(d);
	EXPECT_EQ( count, 25);

	d.cull_unreliable_depths(1.9f, 1.1f);

	// Count how many cells have non-0 depths.
	count = count_non_zero_cells(d);

	EXPECT_EQ( count, 25);
}

void dump_normals (const std::vector<std::vector<std::vector<float>>>& normals) {
	for( int row = 0; row< normals.size(); row++ ) {
		for( int col = 0; col < normals[row].size(); col++) {
			std::cout << "[" << row << "][" << col << "]"
			          << normals[row][col][0] << ", "
					  << normals[row][col][1] << ", "
					  << normals[row][col][2] << std::endl;
		}
	}
}

TEST_F( TestDepthMap, CentralNormalIsZ ) {
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
    d.compute_normals(get_camera(), PLANAR);
	std::vector<std::vector<NormalWithType>> normals = d.get_normals();
	NormalWithType norm = normals.at(3).at(3);

	EXPECT_EQ( norm.x, 0.0f);
	EXPECT_EQ( norm.y, 0.0f);
	EXPECT_EQ( norm.z, 1.0f);
}

TEST_F( TestDepthMap, TopLeftNormalIsNotThere ) {
	// 7x7 map with 5x5 set in centre.
	DepthMap d{"depthmap_test_data/solid_block_test.dat"};
	std::vector<std::vector<NormalWithType>> normals = d.get_normals();

	//-0, 0.948683, 0.316228
	EXPECT_EQ( normals[0][0].x, 0.0f);
	EXPECT_EQ( normals[0][0].y, 0.0f);
	EXPECT_EQ( normals[0][0].z, 0.0f);
}

TEST_F( TestDepthMap, TopCentralNormalIsZ ) {
	// 7x7 map with 5x5 set in centre.
	DepthMap d{"depthmap_test_data/solid_block_test.dat"};
	std::vector<std::vector<NormalWithType>> normals = d.get_normals();

    EXPECT_EQ( normals[1][3].x, 0.0f);
	EXPECT_EQ( normals[1][3].y, 0.0f);
	EXPECT_EQ( normals[1][3].z, 1.0f);
}

TEST_F( TestDepthMap, LeftCentralNormalIsZ ) {
	// 7x7 map with 5x5 set in centre.
	DepthMap d{"depthmap_test_data/solid_block_test.dat"};
	std::vector<std::vector<NormalWithType>> normals = d.get_normals();

	EXPECT_EQ( normals[1][3].x, 0.0f);
	EXPECT_EQ( normals[1][3].y, 0.0f);
	EXPECT_EQ( normals[1][3].z, 1.0f);
}