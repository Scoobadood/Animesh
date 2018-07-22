#include "TestFieldElement.h"

#include <Field/FieldElement.h>
#include "gtest/gtest.h"


void TestFieldElement::SetUp( ) {}
void TestFieldElement::TearDown( ) {}

using namespace animesh;

TEST_F(TestFieldElement, twoArgConstructWithNonUnitNormalShouldThrow) { 
	try {
		FieldElement fe{ vec_0_0_0, vec_1_1_1};
        FAIL( ) << "Expected std::invalid_argument";
	} 
    catch ( std::invalid_argument const & err ) {
        EXPECT_EQ( err.what(), std::string( "Normal should be unit length") );
	}
	catch( ... ) {
        FAIL( ) << "Expected std::invalid_argument";
	}
}

TEST_F(TestFieldElement, passingTest) { 
	SUCCEED();
}

TEST_F(TestFieldElement, twoArgConstructWithUnitNormalShouldGenerateUnitTangent) { 
	FieldElement fe{ vec_0_0_0, vec_1_0_0};

	EXPECT_FLOAT_EQ( 1.0, fe.tangent().norm() );
}

TEST_F(TestFieldElement, twoArgConstructWithUnitNormalShouldGeneratePerpTangent) { 
	FieldElement fe{ vec_0_0_0, vec_1_0_0};

	EXPECT_FLOAT_EQ( 0.0, fe.tangent().dot( fe.normal() ) );
}

TEST_F(TestFieldElement, threeArgConstructWithNonUnitNormalShouldThrow) { 
	try {
		FieldElement fe{ vec_0_0_0, vec_1_1_1, vec_1_0_0};
        FAIL( ) << "Expected std::invalid_argument";
	} 
    catch ( std::invalid_argument const & err ) {
        EXPECT_EQ( err.what(), std::string( "Normal should be unit length") );
	}
	catch( ... ) {
        FAIL( ) << "Expected std::invalid_argument";
	}
}

TEST_F(TestFieldElement, threeArgConstructWithNonUnitTangentShouldThrow) { 
	try {
		FieldElement fe{ vec_0_0_0, vec_1_0_0, vec_1_1_1};
        FAIL( ) << "Expected std::invalid_argument";
	} 
    catch ( std::invalid_argument const & err ) {
        EXPECT_EQ( err.what(), std::string( "Tangent should be unit length") );
	}
	catch( ... ) {
        FAIL( ) << "Expected std::invalid_argument";
	}
}

TEST_F(TestFieldElement, threeArgConstructWithNonPerpTangentShouldThrow) { 
	try {
		FieldElement fe{ vec_0_0_0, vec_1_0_0, vec_1_0_0};
        FAIL( ) << "Expected std::invalid_argument";
	} 
    catch ( std::invalid_argument const & err ) {
        EXPECT_EQ( err.what(), std::string( "Tangent and normal should be perpendicular") );
	}
	catch( ... ) {
        FAIL( ) << "Expected std::invalid_argument";
	}
}