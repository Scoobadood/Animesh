#include "TestRoSy.h"
#include <RoSy/RoSy.h>


static void expectVectorsAreNear(const Eigen::Vector3f& a, const Eigen::Vector3f& b, float delta = 1e-3) {
    if(    (std::abs(a[0]-b[0]) < delta) 
        && (std::abs(a[1]-b[1]) < delta) 
        && (std::abs(a[2]-b[2]) < delta) ) {
        SUCCEED();
    }
    else {
        std::cout << "Not near: (" << a[0] << ", " << a[1] << ", " << a[2] << ") and\n"
                  << "          (" << b[0] << ", " << b[1] << ", " << b[2] << ")"<< std::endl;
        FAIL();
    }
}

void TestProperties::SetUp( ) {}
void TestProperties::TearDown( ) {}


/* ********************************************************************************
 * *
 * *  Test average rosy vectors
 * *   
 * ********************************************************************************/
TEST_F(TestProperties, AverageVectorsFirstWeightZeroGivesSecondVector) {
    using namespace Eigen;

    Vector3f avg = average_rosy_vectors( 	vec_1_0_0, vec_0_1_0, 0, 
    										vec_0_0_1, vec_0_1_0, 1);
    expectVectorsAreNear( vec_0_0_1, avg);
}

TEST_F(TestProperties, AverageVectorsSecondWeightZeroGivesFirstVector) {
    using namespace Eigen;

    Vector3f avg = average_rosy_vectors(	vec_1_0_0, vec_0_1_0, 1, 
    										vec_0_0_1, vec_0_1_0, 0);
    expectVectorsAreNear( vec_1_0_0, avg);
}

TEST_F(TestProperties, AverageVectorsSharedNormal) {
    using namespace Eigen;

    Vector3f avg = average_rosy_vectors(	vec_1_0_0, vec_0_1_0, 1,
    										vec_0_0_1, vec_0_1_0, 1);
    expectVectorsAreNear( std::sqrt(2) * vec_1_0_1, avg);
}

