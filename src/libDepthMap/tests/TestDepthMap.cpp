#include "TestDepthMap.h"

#include <DepthMap/DepthMap.h>
#include <iostream>

void TestDepthMap::SetUp( ) {}
void TestDepthMap::TearDown() {}


/* ********************************************************************************
 * ** Test DepthMap construction from file
 * ********************************************************************************/
TEST_F( TestDepthMap, FileMissingShouldThrow ) {
	try {
		DepthMap d{"missing_file.png"};
		FAIL() << "Expected std::invalid_argument";
	}
	catch ( std::domain_error const & err ) {
		EXPECT_EQ( err.what(), std::string( "DEPTHMAP::MISSING_FILE") );
	}
	catch ( ... ) {
		FAIL( ) << "Expected std::invalid_argument";
	}
}

