#include "TestGeom.h"
#include <Geom/geom.h>
#include <Eigen/Geometry>
#include <iostream>

void TestGeom::SetUp( ) {
    using namespace Eigen;

    BaseP.push_back( Vector3f{ 1, 0, 0} );
    BaseP.push_back( Vector3f{ 0, 1, 0} );
    BaseP.push_back( Vector3f{ 0, 0, 1} );
    BaseP.push_back( Vector3f{ 1, 1, 0} );
    BaseP.push_back( Vector3f{ 1, 0, 1} );
    BaseP.push_back( Vector3f{ 0, 1, 1} );
    BaseP.push_back( Vector3f{ 1, 1, 1} );
    BaseP.push_back( Vector3f{ 3, 4, 5} );
    BaseP.push_back( Vector3f{ -2, -9, -0.5} );
    BaseP.push_back( Vector3f{ 0, 0, 0} );
}
void TestGeom::TearDown() {}

const float EPSILON = 0.1;

void ExpectVectorsAreNear(const Eigen::Vector3f& a, const Eigen::Vector3f& b, float delta) {
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

/* ********************************************************************************
 * ** Test skew symmetric matrix construction
 * ********************************************************************************/
TEST_F( TestGeom, SkewSymmetricMatrixShouldBeCorrect ) {
    using namespace Eigen;

    Vector3f v{ 1, 2, 3};
    Matrix3f m = skew_symmetrix_matrix_for( v );
    EXPECT_FLOAT_EQ( 0, m(0,0) );
    EXPECT_FLOAT_EQ( 0, m(1,1) );
    EXPECT_FLOAT_EQ( 0, m(2,2) );

    EXPECT_FLOAT_EQ(  1, m(2,1) );
    EXPECT_FLOAT_EQ( -1, m(1,2) );

    EXPECT_FLOAT_EQ(  2, m(0,2) );
    EXPECT_FLOAT_EQ( -2, m(2,0) );

    EXPECT_FLOAT_EQ(  3, m(1,0) );
    EXPECT_FLOAT_EQ( -3, m(0,1) );
}

/* ********************************************************************************
 * ** Test computing a perpendicular vector
 * ********************************************************************************/
TEST_F( TestGeom, VectorPerpendicularToZeroShouldThrow ) {
    using namespace Eigen;

    try {
        vector_perpendicular_to_vector( Vector3f::Zero() );
        FAIL() << "Expected std::invalid_argument";
    }
    catch ( std::invalid_argument const & err ) {
        EXPECT_EQ( err.what(), std::string( "Vector may not be zero length") );
    }
    catch ( ... ) {
        FAIL( ) << "Expected std::invalid_argument";
    }
}

TEST_F( TestGeom, VectorPerpendicularTo_1_0_0_is_perpendicular ) {
    using namespace Eigen;

    Vector3f v = vector_perpendicular_to_vector( vec_1_0_0 );

    EXPECT_FLOAT_EQ( 0, vec_1_0_0.dot(v) );
}

TEST_F( TestGeom, VectorPerpendicularTo_0_1_0_is_perpendicular ) {
    using namespace Eigen;

    Vector3f v = vector_perpendicular_to_vector( vec_0_1_0 );
    EXPECT_FLOAT_EQ( 0, vec_0_1_0.dot(v) );
}


TEST_F( TestGeom, VectorPerpendicularTo_0_0_1_is_perpendicular ) {
    using namespace Eigen;

    Vector3f v = vector_perpendicular_to_vector( vec_0_0_1 );
    EXPECT_FLOAT_EQ( 0, vec_0_0_1.dot(v) );
}

TEST_F( TestGeom, VectorPerpendicularTo_1_1_0_is_perpendicular ) {
    using namespace Eigen;
    Vector3f in = vec_1_0_0 + vec_0_1_0;
    Vector3f v = vector_perpendicular_to_vector( in );
    EXPECT_FLOAT_EQ( 0, in.dot(v) );
}

TEST_F( TestGeom, VectorPerpendicularTo_1_0_1_is_perpendicular ) {
    using namespace Eigen;
    Vector3f in = vec_1_0_0 + vec_0_0_1;
    Vector3f v = vector_perpendicular_to_vector( in );
    EXPECT_FLOAT_EQ( 0, in.dot(v) );
}

TEST_F( TestGeom, VectorPerpendicularTo_0_1_1_is_perpendicular ) {
    using namespace Eigen;
    Vector3f in = vec_0_1_0 + vec_0_0_1;
    Vector3f v = vector_perpendicular_to_vector( in );
    EXPECT_FLOAT_EQ( 0, in.dot(v) );
}

TEST_F( TestGeom, VectorPerpendicularTo_1_1_1_is_perpendicular ) {
    using namespace Eigen;
    Vector3f in = vec_0_1_0 + vec_0_0_1 + vec_1_0_0;
    Vector3f v = vector_perpendicular_to_vector( in );
    EXPECT_FLOAT_EQ( 0, in.dot(v) );
}

TEST_F( TestGeom, VectorPerpendicularToManyIsActuallyPerpendicular ) {
    using namespace Eigen;

    Vector3f in; in << 7, -12, 3.7;
    Vector3f out = vector_perpendicular_to_vector( in );

    EXPECT_FLOAT_EQ( 0, in.dot(out) );
}

/* ********************************************************************************
 * ** Test V2V Rotation Works
 * ********************************************************************************/
TEST_F( TestGeom, Vector2VectorShouldThrowIfFirstVector0 ) {
    using namespace Eigen;

    try {
        vector_to_vector_rotation( Vector3f::Zero(), vec_0_1_0 );
        FAIL() << "Expected std::invalid_argument";
    }
    catch ( std::invalid_argument const & err ) {
        EXPECT_EQ( err.what(), std::string( "Vector may not be zero length") );
    }
    catch ( ... ) {
        FAIL( ) << "Expected std::invalid_argument";
    }
}

TEST_F( TestGeom, Vector2VectorShouldThrowIfSecondVector0 ) {
    using namespace Eigen;

    try {
        vector_to_vector_rotation( vec_0_1_0, Vector3f::Zero() );
        FAIL() << "Expected std::invalid_argument";
    }
    catch ( std::invalid_argument const & err ) {
        EXPECT_EQ( err.what(), std::string( "Vector may not be zero length") );
    }
    catch ( ... ) {
        FAIL( ) << "Expected std::invalid_argument";
    }
}

TEST_F( TestGeom, RotateVectorsAlignXToY ) {
    using namespace Eigen;

    Matrix3f m = vector_to_vector_rotation( vec_1_0_0, vec_0_1_0 );

    Vector3f should_be_0_1_0 = m * vec_1_0_0;
    ExpectVectorsAreNear(vec_0_1_0, should_be_0_1_0, EPSILON );
}

TEST_F( TestGeom, RotateVectorsAlignXToZ ) {
    using namespace Eigen;

    Matrix3f m = vector_to_vector_rotation( vec_1_0_0, vec_0_0_1 );

    Vector3f should_be_0_0_1 = m * vec_1_0_0;

    ExpectVectorsAreNear(vec_0_0_1, should_be_0_0_1, EPSILON );
}

TEST_F( TestGeom, RotateVectorsAlignYToZ ) {
    using namespace Eigen;

    Matrix3f m = vector_to_vector_rotation( vec_0_1_0, vec_0_0_1 );

    Vector3f should_be_0_0_1 = m * vec_0_1_0;

    ExpectVectorsAreNear(vec_0_0_1, should_be_0_0_1, EPSILON );
}

TEST_F( TestGeom, RotateVectorsAlignZToY ) {
    using namespace Eigen;

    Matrix3f m = vector_to_vector_rotation( vec_0_0_1, vec_0_1_0 );

    Vector3f should_be_0_1_0 = m * vec_0_0_1;

    ExpectVectorsAreNear(vec_0_1_0, should_be_0_1_0, EPSILON );
}

TEST_F( TestGeom, RotateVectorsAlignZToX ) {
    using namespace Eigen;

    Matrix3f m = vector_to_vector_rotation( vec_0_0_1, vec_1_0_0 );

    Vector3f should_be_1_0_0 = m * vec_0_0_1;

    ExpectVectorsAreNear(vec_1_0_0, should_be_1_0_0, EPSILON );
}

TEST_F( TestGeom, RotateVectorsAlignYToX ) {
    using namespace Eigen;

    Matrix3f m = vector_to_vector_rotation( vec_0_1_0, vec_1_0_0 );

    Vector3f should_be_1_0_0 = m * vec_0_1_0;

    ExpectVectorsAreNear(vec_1_0_0, should_be_1_0_0, EPSILON );
}

TEST_F( TestGeom, RotateVectorsAlignRandomToRandom ) {
    using namespace Eigen;

    Vector3f v1 = Vector3f::Random( ) * 10;
    Vector3f v2 = Vector3f::Random( ) * -5;

    Matrix3f m = vector_to_vector_rotation( v1, v2 );

    float scale_factor = v2.norm() / v1.norm();
    Vector3f should_be_v2 = m * v1 * scale_factor;

    ExpectVectorsAreNear(v2, should_be_v2, EPSILON );
}

TEST_F( TestGeom, RotateSameVectorsShouldReturnIdentity ) {
    using namespace Eigen;

    Matrix3f m = vector_to_vector_rotation( vec_1_0_0, vec_1_0_0 );

    EXPECT_FLOAT_EQ( 1, m(0) );
    EXPECT_FLOAT_EQ( 1, m(4) );
    EXPECT_FLOAT_EQ( 1, m(8) );
    EXPECT_FLOAT_EQ( 0, m(1) );
    EXPECT_FLOAT_EQ( 0, m(2) );
    EXPECT_FLOAT_EQ( 0, m(3) );
    EXPECT_FLOAT_EQ( 0, m(5) );
    EXPECT_FLOAT_EQ( 0, m(6) );
    EXPECT_FLOAT_EQ( 0, m(7) );
}

TEST_F( TestGeom, RotateOpposingVectorsShouldReturn ) {
    using namespace Eigen;

    Matrix3f m = vector_to_vector_rotation( vec_1_0_0, - vec_1_0_0 );

    Vector3f should_be_m1_0_0 = m * vec_1_0_0;

    ExpectVectorsAreNear(Vector3f{-1,0,0}, should_be_m1_0_0, EPSILON );
}

/* ********************************************************************************
 * ** Test General rotation works
 * ********************************************************************************/

void setup_p1_p2( tP1P2_Mode mode, Eigen::Vector3f& P1, Eigen::Vector3f& P2 ) {
    switch( mode ) {
        case PT_XY_PLANE:
            P1 = Eigen::Vector3f{ 5, 2, 0 };
            P2 = Eigen::Vector3f{ -4, 6, 0 };
            break;
        case PT_3D:
            P1 = Eigen::Vector3f{ 5, 2, 3 };
            P2 = Eigen::Vector3f{ -4, 6, -3.4 };
            break;
        case PT_ZERO:
            P1 = Eigen::Vector3f::Zero();
            P2 = P1;
            break;
        case PT_XY_PLANE_COLO:
            P1 = Eigen::Vector3f{ 5, 2, 0 };
            P2 = P1;
            break;
        case PT_3D_COLO:
            P1 = Eigen::Vector3f{ 5, 2, 3 };
            P2 = P1;
            break;
        }
}

void setup_normal( tNormalsMode mode, Eigen::Vector3f& N1 ) {
    switch( mode ) {
        case NRM_Z_AXIS:
            N1 = Eigen::Vector3f{ 0, 0, 1};
            break;
        case NRM_1_1_1:
            N1 = Eigen::Vector3f{ 1, 1, 1}.normalized();
            break;
    }
}

void setup_rotation( tRotationMode mode, Eigen::Matrix3f& R ) {
    switch( mode ) {
        case ROT_NONE:
            R = Eigen::Matrix3f::Identity();
            break;
        case ROT_20_30_40:
            R << 0.8138, 0.0400, 0.5798, 0.2962, 0.8298, -0.4730, -0.5000, 0.5567, 0.6634;
            break;
        case ROT_20_0_0:
            R << 0.9397, -0.3420, 0, 0.3420, 0.9397, 0, 0, 0, 1.0000;
            break;
        case ROT_0_30_0:
            R << 0.8660, 0, 0.5000, 0, 1.0000, 0, -0.5000, 0, 0.8660;
            break;
        case ROT_0_0_40:
            R << 1.0000, 0, 0, 0, 0.7660, -0.6428, 0, 0.6428, 0.7660;
            break;
    }
}

void TestGeom::init_p_q_n2( std::vector<Eigen::Vector3f>& P, std::vector<Eigen::Vector3f>& Q, 
    const Eigen::Matrix3f& R, 
    const Eigen::Vector3f& P1,
    const Eigen::Vector3f& P2, 
    const Eigen::Vector3f& N1, 
          Eigen::Vector3f& N2 ) {

    for( auto bp : BaseP ) {
        P.push_back( bp + P1 );
        Q.push_back( (R * bp) + P2);
    }
    N2 = R * N1;
}

void TestGeom::check_results( const std::vector<Eigen::Vector3f>& P, 
                    const std::vector<Eigen::Vector3f>& Q, 
                    const Eigen::Vector3f& P1, 
                    const Eigen::Vector3f& P2, 
                    const Eigen::Matrix3f& m ) {
    using namespace Eigen;

    for( size_t i = 0; i<BaseP.size(); ++i ) {
        Vector3f expected_p = P[i];
        Vector3f q = Q[i] - P2;
        Vector3f predicted_p = ( m.transpose() * q ) + P1;

        ExpectVectorsAreNear(expected_p, predicted_p, EPSILON );
    }
}

void TestGeom::run_test( tP1P2_Mode pMode, tNormalsMode nMode, tRotationMode rMode ) {
    using namespace Eigen;

    Vector3f P1;
    Vector3f P2;
    setup_p1_p2( pMode, P1, P2 );

    Vector3f N1;
    setup_normal( nMode, N1 );

    Matrix3f R;
    setup_rotation( rMode, R );

    std::vector<Vector3f> P;
    std::vector<Vector3f> Q;
    Vector3f N2;
    init_p_q_n2( P, Q, R, P1, P2, N1, N2 );

    // Execute the test
    Matrix3f M = rotation_between( P1, N1, P, P2, N2, Q);

    check_results( P, Q, P1, P2, M);
}


TEST_F( TestGeom, Align_P_Zero_N_Z_R_none) {
    run_test( PT_ZERO, NRM_Z_AXIS, ROT_NONE );
}

TEST_F( TestGeom, Align_P_Zero_N_1_1_1_R_none) {
    run_test( PT_ZERO, NRM_1_1_1, ROT_NONE );
}

TEST_F( TestGeom, Align_P_Zero_N_1_1_1_R_20) {
    run_test( PT_ZERO, NRM_1_1_1, ROT_20_0_0 );
}

TEST_F( TestGeom, Align_P_Zero_N_1_1_1_R_30) {
    run_test( PT_ZERO, NRM_1_1_1, ROT_0_30_0 );
}

TEST_F( TestGeom, Align_P_Zero_N_1_1_1_R_40) {
    run_test( PT_ZERO, NRM_1_1_1, ROT_0_0_40 );
}

TEST_F( TestGeom, Align_P_Zero_N_1_1_1_R_20_30_40) {
    run_test( PT_ZERO, NRM_1_1_1, ROT_20_30_40 );
}


TEST_F( TestGeom, Align_P_XY_N_Z_R_none) {
    run_test( PT_XY_PLANE, NRM_Z_AXIS, ROT_NONE );
}

TEST_F( TestGeom, Align_P_XY_N_1_1_1_R_none) {
    run_test( PT_XY_PLANE, NRM_1_1_1, ROT_NONE );
}

TEST_F( TestGeom, Align_P_XY_N_1_1_1_R_20) {
    run_test( PT_XY_PLANE, NRM_1_1_1, ROT_20_0_0 );
}

TEST_F( TestGeom, Align_P_XY_N_1_1_1_R_30) {
    run_test( PT_XY_PLANE, NRM_1_1_1, ROT_0_30_0 );
}

TEST_F( TestGeom, Align_P_XY_N_1_1_1_R_40) {
    run_test( PT_XY_PLANE, NRM_1_1_1, ROT_0_0_40 );
}

TEST_F( TestGeom, Align_P_XY_N_1_1_1_R_20_30_40) {
    run_test( PT_XY_PLANE, NRM_1_1_1, ROT_20_30_40 );
}


TEST_F( TestGeom, Align_P_3D_N_Z_R_none) {
    run_test( PT_3D, NRM_Z_AXIS, ROT_NONE );
}

TEST_F( TestGeom, Align_P_3D_N_1_1_1_R_none) {
    run_test( PT_3D, NRM_1_1_1, ROT_NONE );
}

TEST_F( TestGeom, Align_P_3D_N_1_1_1_R_20) {
    run_test( PT_3D, NRM_1_1_1, ROT_20_0_0 );
}

TEST_F( TestGeom, Align_P_3D_N_1_1_1_R_30) {
    run_test( PT_3D, NRM_1_1_1, ROT_0_30_0 );
}

TEST_F( TestGeom, Align_P_3D_N_1_1_1_R_40) {
    run_test( PT_3D, NRM_1_1_1, ROT_0_0_40 );
}

TEST_F( TestGeom, Align_P_3D_N_1_1_1_R_20_30_40) {
    run_test( PT_3D, NRM_1_1_1, ROT_20_30_40 );
}


TEST_F( TestGeom, Align_P_XYC_N_Z_R_none) {
    run_test( PT_XY_PLANE_COLO, NRM_Z_AXIS, ROT_NONE );
}

TEST_F( TestGeom, Align_P_XYC_N_1_1_1_R_none) {
    run_test( PT_XY_PLANE_COLO, NRM_1_1_1, ROT_NONE );
}

TEST_F( TestGeom, Align_P_XYC_N_1_1_1_R_20) {
    run_test( PT_XY_PLANE_COLO, NRM_1_1_1, ROT_20_0_0 );
}

TEST_F( TestGeom, Align_P_XYC_N_1_1_1_R_30) {
    run_test( PT_XY_PLANE_COLO, NRM_1_1_1, ROT_0_30_0 );
}

TEST_F( TestGeom, Align_P_XYC_N_1_1_1_R_40) {
    run_test( PT_XY_PLANE_COLO, NRM_1_1_1, ROT_0_0_40 );
}

TEST_F( TestGeom, Align_P_XYC_N_1_1_1_R_20_30_40) {
    run_test( PT_XY_PLANE_COLO, NRM_1_1_1, ROT_20_30_40 );
}


TEST_F( TestGeom, Align_P_3DC_N_Z_R_none) {
    run_test( PT_3D_COLO, NRM_Z_AXIS, ROT_NONE );
}

TEST_F( TestGeom, Align_P_3DC_N_1_1_1_R_none) {
    run_test( PT_3D_COLO, NRM_1_1_1, ROT_NONE );
}

TEST_F( TestGeom, Align_P_3DC_N_1_1_1_R_20) {
    run_test( PT_3D_COLO, NRM_1_1_1, ROT_20_0_0 );
}

TEST_F( TestGeom, Align_P_3DC_N_1_1_1_R_30) {
    run_test( PT_3D_COLO, NRM_1_1_1, ROT_0_30_0 );
}

TEST_F( TestGeom, Align_P_3DC_N_1_1_1_R_40) {
    run_test( PT_3D_COLO, NRM_1_1_1, ROT_0_0_40 );
}

TEST_F( TestGeom, Align_P_3DC_N_1_1_1_R_20_30_40) {
    run_test( PT_3D_COLO, NRM_1_1_1, ROT_20_30_40 );
}


TEST_F( TestGeom, Align_P_3D_N_Z_R_20) {
    run_test( PT_3D, NRM_Z_AXIS, ROT_20_0_0 );
}

TEST_F( TestGeom, Align_P_3D_N_Z_R_30) {
    run_test( PT_3D, NRM_Z_AXIS, ROT_0_30_0 );
}

TEST_F( TestGeom, Align_P_3D_N_Z_R_40) {
    run_test( PT_3D, NRM_Z_AXIS, ROT_0_0_40 );
}

TEST_F( TestGeom, Align_P_3D_N_Z_R_20_30_40) {
    run_test( PT_3D, NRM_Z_AXIS, ROT_20_30_40 );
}