#include "TestPoSy.h"
#include <PoSy/PoSy.h>

void TestPoSy::SetUp( ) {}
void TestPoSy::TearDown( ) {}

/* ********************************************************************************
 * *
 * *  Test average rosy vectors
 * *   
 * ********************************************************************************/
TEST_F(TestPoSy, MissingPointsShouldThrow) {
    using namespace Eigen;

    FAIL();
}