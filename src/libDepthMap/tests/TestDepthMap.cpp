#include "TestDepthMap.h"

#include <DepthMap/DepthMap.h>
#include <iostream>

void TestDepthMap::SetUp( ) {}
void TestDepthMap::TearDown() {}


/* ********************************************************************************
 * ** Test DepthMap construction from file
 * ********************************************************************************/
TEST_F( TestDepthMap, ShouldParseDimensions ) {
	DepthMap d{"depthmap_test_data/dm.pgm"};
	EXPECT_EQ( d.cols(), 640);
	EXPECT_EQ( d.rows(), 480);
}

