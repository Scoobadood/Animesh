#include "TestDepthMap.h"

#include <iostream>
#include <cmath>
const float INV_SQRT_2 = 1.0f / std::sqrt(2.0f);

void TestDepthMap::SetUp( ) {}
void TestDepthMap::TearDown() {}
unsigned int TestDepthMap::count_non_zero_cells(DepthMap& d) {
	unsigned int count = 0;
	for( int r = 0; r < d.rows(); ++r) {
		for( int c = 0; c < d.cols(); ++c ) {
			if( d.depth_at(r,c) > 0.0f ) {
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
	for( int r = 0; r < d.rows(); r++ ) {
		for( int c = 0; c < d.cols(); c++ ) {
			std::cout << d.depth_at(r,c) << "  ";
		}
		std::cout << std::endl;
	}
}

/* ********************************************************************************
 * ** Test DepthMap construction from file
 * ********************************************************************************/
TEST_F( TestDepthMap, ShouldParseDimensions ) {
	DepthMap d{"depthmap_test_data/dm.dat"};
	EXPECT_EQ( d.cols(), 640);
	EXPECT_EQ( d.rows(), 480);
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
	// 7x7 map with 5x5 set in centre.
	DepthMap d{"depthmap_test_data/solid_block_test.dat"};
	std::vector<std::vector<std::vector<float>>> normals = d.get_normals();

	EXPECT_EQ( normals[3][3][0], 0.0f);
	EXPECT_EQ( normals[3][3][1], 0.0f);
	EXPECT_EQ( normals[3][3][2], 1.0f);
}

TEST_F( TestDepthMap, TopLeftNormalIsNotThere ) {
	// 7x7 map with 5x5 set in centre.
	DepthMap d{"depthmap_test_data/solid_block_test.dat"};
	std::vector<std::vector<std::vector<float>>> normals = d.get_normals();

	//-0, 0.948683, 0.316228
	EXPECT_EQ( normals[0][0][0], 0.0f);
	EXPECT_EQ( normals[0][0][1], 0.0f);
	EXPECT_EQ( normals[0][0][2], 0.0f);
}

TEST_F( TestDepthMap, TopCentralNormalIsZ ) {
	// 7x7 map with 5x5 set in centre.
	DepthMap d{"depthmap_test_data/solid_block_test.dat"};
	std::vector<std::vector<std::vector<float>>> normals = d.get_normals();

    EXPECT_EQ( normals[1][3][0], 0.0f);
	EXPECT_EQ( normals[1][3][1], 0.0f);
	EXPECT_EQ( normals[1][3][2], 1.0f);
}

TEST_F( TestDepthMap, LeftCentralNormalIsZ ) {
	// 7x7 map with 5x5 set in centre.
	DepthMap d{"depthmap_test_data/solid_block_test.dat"};
	std::vector<std::vector<std::vector<float>>> normals = d.get_normals();

	EXPECT_EQ( normals[3][1][0], 0.0f);
	EXPECT_EQ( normals[3][1][1], 0.0f);
	EXPECT_EQ( normals[3][1][2], 1.0f);
}