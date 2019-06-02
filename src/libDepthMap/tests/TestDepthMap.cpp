#include "TestDepthMap.h"

#include <iostream>

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

	dump(d);

	EXPECT_EQ( count, 20);
}

TEST_F( TestDepthMap, VerticalEdgeTest ) {
	DepthMap d{"depthmap_test_data/vertical_edge_test.dat"};
	unsigned int count = count_non_zero_cells(d);
	EXPECT_EQ( count, 25);

	d.cull_unreliable_depths(1.9f, 0.5f);

	// Count how many cells have non-0 depths.
	count = count_non_zero_cells(d);

	dump(d);

	EXPECT_EQ( count, 20);
}

TEST_F( TestDepthMap, SolidBlockTest ) {
	DepthMap d{"depthmap_test_data/solid_block_test.dat"};
	unsigned int count = count_non_zero_cells(d);
	EXPECT_EQ( count, 25);

	d.cull_unreliable_depths(2.0f, 0.5f);

	// Count how many cells have non-0 depths.
	count = count_non_zero_cells(d);

	dump(d);

	EXPECT_EQ( count, 25);
}

TEST_F( TestDepthMap, StepEdgeTest ) {
	DepthMap d{"depthmap_test_data/step_test.dat"};
	unsigned int count = count_non_zero_cells(d);
	EXPECT_EQ( count, 25);

	d.cull_unreliable_depths(2.0f, 0.5f);

	// Count how many cells have non-0 depths.
	count = count_non_zero_cells(d);

	dump(d);

	EXPECT_EQ( count, 25);
}

TEST_F( TestDepthMap, NoisyCentreTest ) {
	DepthMap d{"depthmap_test_data/noisy_centre_test.dat"};
	unsigned int count = count_non_zero_cells(d);
	EXPECT_EQ( count, 25);

	d.cull_unreliable_depths(2.0f, 0.5f);

	// Count how many cells have non-0 depths.
	count = count_non_zero_cells(d);

	dump(d);

	EXPECT_EQ( count, 25);
}

TEST_F( TestDepthMap, DiagonalEdgeTest ) {
	DepthMap d{"depthmap_test_data/diagonal_edge_test.dat"};
	unsigned int count = count_non_zero_cells(d);
	EXPECT_EQ( count, 25);

	d.cull_unreliable_depths(1.9f, 1.1f);

	// Count how many cells have non-0 depths.
	count = count_non_zero_cells(d);

	dump(d);

	EXPECT_EQ( count, 25);
}

