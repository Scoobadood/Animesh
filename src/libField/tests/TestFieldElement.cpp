#include "TestFieldElement.h"

#include <Field/FieldElement.h>
#include "gtest/gtest.h"


void TestFieldElement::SetUp( ) {}
void TestFieldElement::TearDown( ) {}

using namespace animesh;

#define EXPECT_EQ_VEC(vec1, vec2) \
	EXPECT_FLOAT_EQ(vec1[0], vec2[0]); \
	EXPECT_FLOAT_EQ(vec1[1], vec2[1]); \
	EXPECT_FLOAT_EQ(vec1[2], vec2[2]);

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

TEST_F(TestFieldElement, mergeIdenticalVectorsShouldGiveSameFE) { 
	const FieldElement * fe1 = new FieldElement( vec_0_0_0, vec_0_1_0, vec_0_0_1);

	FieldElement * fe2 = FieldElement::mergeFieldElements ( fe1, fe1 );

	EXPECT_EQ_VEC( vec_0_0_0, fe2->location());
	EXPECT_EQ_VEC( vec_0_1_0, fe2->normal());

	// Tangent should be in plane
	EXPECT_FLOAT_EQ( 0.0f, fe2->tangent()[1]);
	EXPECT_FLOAT_EQ( 1.0f, fe2->tangent().norm());
}

TEST_F(TestFieldElement, mergeVectorsWithCommonNormalShouldHaveSameNormal) { 
	const FieldElement * fe1 = new FieldElement( vec_0_0_0, vec_0_1_0, vec_0_0_1);
	const FieldElement * fe2 = new FieldElement( vec_1_0_0, vec_0_1_0, vec_0_0_1);

	FieldElement * fe3 = FieldElement::mergeFieldElements ( fe1, fe2 );

	Eigen::Vector3f mid{0.5f, 0.0f, 0.0f};
	EXPECT_EQ_VEC( mid, fe3->location());
	EXPECT_EQ_VEC( vec_0_1_0, fe3->normal());

	// Tangent should be in plane
	EXPECT_FLOAT_EQ( 0.0f, fe2->tangent()[1]);
	EXPECT_FLOAT_EQ( 1.0f, fe2->tangent().norm());
}