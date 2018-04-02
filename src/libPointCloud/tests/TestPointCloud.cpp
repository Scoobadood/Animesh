#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "TestPointCloud.h"

#include <Eigen/Core>

void TestPointCloud::SetUp( ) {}
void TestPointCloud::TearDown( ) {}


TEST_F(TestPointCloud, InitialSizeIsZero) { 
	PointCloud * test_pc = new PointCloud( );

	size_t actual_size = test_pc->size();

	EXPECT_EQ( 0, actual_size);
}


TEST_F(TestPointCloud, AddingAPointShouldIncreaseSize) { 
	PointCloud * test_pc = new PointCloud( );
	test_pc->add_point( Eigen::Vector3f{ 1.1, 2.2, 3.3} );

	size_t actual_size = test_pc->size();

	EXPECT_EQ( 1, actual_size);
}

TEST_F(TestPointCloud, ComputeNormalsShouldWork) { 
	PointCloud * test_pc = new PointCloud( );

	float maxTheta		= 2 * M_PI;
	float maxPhi 		= M_PI;
	float deltaTheta 	= maxTheta / 20.0f;
	float deltaPhi   	= maxPhi / 20.0f;
	float radius		= 1.0f;

	for( float theta = 0.0f; theta < maxTheta; theta +=  deltaTheta) {
		for( float phi = deltaPhi; phi<maxPhi; phi += deltaPhi ) {
			Eigen::Vector3f location { radius * sin(phi) * cos(theta),   radius * cos( phi ), radius * sin(phi) * sin(theta) };

			test_pc->add_point( location );
		}
	}
	test_pc->compute_normals( );

	for( size_t i=0; i < test_pc->size(); ++i ) {
		Point p = test_pc->point( i );

		EXPECT_FLOAT_EQ( p.location[0], p.normal[0] );
		EXPECT_FLOAT_EQ( p.location[1], p.normal[1] );
		EXPECT_FLOAT_EQ( p.location[2], p.normal[2] );
	}
}
