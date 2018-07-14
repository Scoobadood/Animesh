#pragma once

#include <Eigen/Core>
#include <vector>
#include "gtest/gtest.h"

typedef enum {
    PT_ZERO,
    PT_XY_PLANE,
    PT_XY_PLANE_COLO,
    PT_3D,
    PT_3D_COLO
} tP1P2_Mode;

typedef enum {
    NRM_Z_AXIS,
    NRM_1_1_1
} tNormalsMode;

typedef enum {
    ROT_NONE,
    ROT_20_0_0,
    ROT_0_30_0,
    ROT_0_0_40,
    ROT_20_30_40
} tRotationMode;

class TestGeom : public ::testing::Test {
public:
	Eigen::Vector3f vec_1_0_0{ 1.0f, 0.0f, 0.0f };
	Eigen::Vector3f vec_0_1_0{ 0.0f, 1.0f, 0.0f };
	Eigen::Vector3f vec_0_0_1{ 0.0f, 0.0f, 1.0f };
	Eigen::Vector3f vec_0_1_R3{ 0.0f, 1.0f, sqrt( 3.0f ) };
	Eigen::Vector3f vec_0_R3_1{ 0.0f, sqrt( 3.0f ), 1.0f };

	std::vector<Eigen::Vector3f> BaseP;

	void SetUp( );
	void TearDown();
	void init_p_q_n2( std::vector<Eigen::Vector3f>& P, std::vector<Eigen::Vector3f>& Q, 
    	const Eigen::Matrix3f& R, 
    	const Eigen::Vector3f& P1,
    	const Eigen::Vector3f& P2, 
    	const Eigen::Vector3f& N1, 
        	  Eigen::Vector3f& N2 );
	void run_test( tP1P2_Mode pMode, tNormalsMode nMode, tRotationMode rMode );
	void check_results( const std::vector<Eigen::Vector3f>& P, 
                    const std::vector<Eigen::Vector3f>& Q, 
                    const Eigen::Vector3f& P1, 
                    const Eigen::Vector3f& P2, 
                    const Eigen::Matrix3f& m );


};
