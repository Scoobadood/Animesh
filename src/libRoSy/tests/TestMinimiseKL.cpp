#include "TestMinimiseKL.h"
#include <RoSy/RoSy.h>

void TestMinimiseKL::SetUp( ) {}
void TestMinimiseKL::TearDown() {}


/*
 * Test the function to return k,l that minimise the inter vectr angle
 */
/* ********************************************************************************
 * ** Test computing the smallest rotations
 * ********************************************************************************/

TEST_F( TestMinimiseKL, ShouldThrowIfN1IsNotUnitVector ) {
    using namespace Eigen;

    Vector3f o1{ 0.0f, 2.0f, 3.0f };
    Vector3f n1{ 1.0f, 1.0f, 0.0f };
    Vector3f o2{ 0.0f, 2.0f, 3.0f };
    Vector3f n2 = vec_1_0_0;

    int actualK;
    int actualL;

    try {
        best_rosy_vector_and_kl( o1, n1, actualK, o2, n2, actualL );
        FAIL() << "Expected std::invalid_argument";
    }
    catch ( std::invalid_argument const & err ) {
        EXPECT_EQ( err.what(), std::string( "Normal must be unit vector") );
    }
    catch ( ... ) {
        FAIL( ) << "Expected std::invalid_argument";
    }
}

TEST_F( TestMinimiseKL, ShouldThrowIfO1IsZero ) {
    using namespace Eigen;

    Vector3f o1{ 0.0f, 0.0f, 0.0f };
    Vector3f n1 = vec_1_0_0;
    Vector3f o2{ 0.0f, 2.0f, 3.0f };
    Vector3f n2 = vec_1_0_0;

    int actualK;
    int actualL;

    try {
        best_rosy_vector_and_kl( o1, n1, actualK, o2, n2, actualL );
        FAIL() << "Expected std::invalid_argument";
    }
    catch ( std::invalid_argument const & err ) {
        EXPECT_EQ( err.what(), std::string( "Vector may not be zero length") );
    }
    catch ( ... ) {
        FAIL( ) << "Expected std::invalid_argument";
    }
}

TEST_F( TestMinimiseKL, ShouldThrowIfN2IsNotUnitVector ) {
    using namespace Eigen;

    Vector3f o1{ 0.0f, 2.0f, 3.0f };
    Vector3f n1{ 1.0f, 1.0f, 0.0f };
    Vector3f o2{ 0.0f, 2.0f, 3.0f };
    Vector3f n2{ 1.0f, 1.0f, 0.0f };

    int actualK;
    int actualL;

    try {
        best_rosy_vector_and_kl( o1, n1, actualK, o2, n2, actualL );
        FAIL() << "Expected std::invalid_argument";
    }
    catch ( std::invalid_argument const & err ) {
        EXPECT_EQ( err.what(), std::string( "Normal must be unit vector") );
    }
    catch ( ... ) {
        FAIL( ) << "Expected std::invalid_argument";
    }
}

TEST_F( TestMinimiseKL, ShouldThrowIfO2IsZero ) {
    using namespace Eigen;

    Vector3f o1{ 0.0f, 2.0f, 3.0f };
    Vector3f n1 = vec_1_0_0;
    Vector3f o2{ 0.0f, 0.0f, 0.0f };
    Vector3f n2 = vec_1_0_0;

    int actualK;
    int actualL;

    try {
        best_rosy_vector_and_kl( o1, n1, actualK, o2, n2, actualL );
        FAIL() << "Expected std::invalid_argument";
    }
    catch ( std::invalid_argument const & err ) {
        EXPECT_EQ( err.what(), std::string( "Vector may not be zero length") );
    }
    catch ( ... ) {
        FAIL( ) << "Expected std::invalid_argument";
    }
}


/* ***
 * *
 * */
TEST_F( TestMinimiseKL, ShouldBe_0_0_For_0_DegreesCoplanar ) {
    using namespace Eigen;

    Vector3f o1 = vec_0_1_0;
    Vector3f n1 = vec_1_0_0;
    Vector3f o2 = vec_0_1_0;
    Vector3f n2 = vec_1_0_0;

    int actualK;
    int actualL;

    int expectedK = 0;
    int expectedL = 0;

    Vector3f best = best_rosy_vector_and_kl( o1, n1, actualK, o2, n2, actualL );

    EXPECT_EQ( expectedK, actualK );
    EXPECT_EQ( expectedL, actualL );
}

TEST_F( TestMinimiseKL, ShouldBe_0_0_For_30_DegreesCoplanar ) {
    using namespace Eigen;

    Vector3f o1{ 0.0f, sqrt( 3.0f ), 1.0f };
    Vector3f n1 = vec_1_0_0;
    Vector3f o2 = vec_0_1_0;
    Vector3f n2 = vec_1_0_0;

    int actualK;
    int actualL;

    int expectedK = 0;
    int expectedL = 0;
    float expectedTheta = M_PI / 6.0f;

    Vector3f best = best_rosy_vector_and_kl( o1, n1, actualK, o2, n2, actualL );

    EXPECT_EQ( expectedK, actualK );
    EXPECT_EQ( expectedL, actualL );
    
}

TEST_F( TestMinimiseKL, ShouldBe_0_0_For_45_DegreesCoplanar ) {
    using namespace Eigen;

    Vector3f o1{ 0.0f, 1.0f, 1.0f };
    Vector3f n1 = vec_1_0_0;
    Vector3f o2 = vec_0_1_0;
    Vector3f n2 = vec_1_0_0;

    int actualK;
    int actualL;

    int expectedK = 0;
    int expectedL = 0;
    float expectedTheta = M_PI / 4.0f;

    Vector3f best = best_rosy_vector_and_kl( o1, n1, actualK, o2, n2, actualL );

    EXPECT_EQ( expectedK, actualK );
    EXPECT_EQ( expectedL, actualL );
    
}


TEST_F( TestMinimiseKL, ShouldBe_0_1_For_60_DegreesCoplanar ) {
    using namespace Eigen;

    Vector3f o1 = vec_0_1_R3;
    Vector3f n1 = vec_1_0_0;
    Vector3f o2 = vec_0_1_0;
    Vector3f n2 = vec_1_0_0;

    int actualK;
    int actualL;

    int expectedK = 0;
    int expectedL = 1;
    float expectedTheta = M_PI / 6.0f;

    Vector3f best = best_rosy_vector_and_kl( o1, n1, actualK, o2, n2, actualL );

    EXPECT_EQ( expectedK, actualK );
    EXPECT_EQ( expectedL, actualL );
    
}

TEST_F( TestMinimiseKL, SPROG ) {
    using namespace Eigen;

    Vector3f o1{ 1.0f, 0.5f, 0.0f };
    Vector3f n1{ 0.0f, 0.0f, 1.0f };
    Vector3f o2{ -0.5f, 1.0f, 0.0f };
    Vector3f n2{ 0.0f, 1.0f, 0.0f };

    int actualK;
    int actualL;
    Vector3f best = best_rosy_vector_and_kl( o1, n1, actualK, o2, n2, actualL);
}