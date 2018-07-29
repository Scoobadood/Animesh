#include "TestField.h"

#include <Field/Field.h>
#include <Field/FieldElement.h>


void TestField::SetUp( ) {}
void TestField::TearDown( ) {}

TEST_F(TestField, findCorrespondenceWithDifferentSizeVectorsShouldThrow) { 
	using namespace std;
	using namespace animesh;

	vector<FieldElement * > first;
	vector<FieldElement * > second;

	FieldElement *fe1 = new FieldElement( vec_0_0_0, vec_1_0_0);
	FieldElement *fe2 = new FieldElement( vec_0_0_0, vec_1_0_0);
	FieldElement *fe3 = new FieldElement( vec_0_0_0, vec_1_0_0);

	first.push_back( fe1 );
	first.push_back( fe2 );
	second.push_back( fe3 );

	Field::Correspondence corr;

	try {
		find_correspondences( first, second, corr );
        FAIL() << "Expected std::invalid_argument";
	} catch( invalid_argument const & err) {
        EXPECT_EQ( err.what(), std::string( "find_correspondences expects vectors to be the same size") );
	} catch( ... ) {
        FAIL() << "Expected std::invalid_argument";
	}
}

TEST_F(TestField, findCorrespondenceWithZeroFirstVectorShouldThrow) { 
	using namespace std;
	using namespace animesh;

	vector<FieldElement * > first;
	vector<FieldElement * > second;

	FieldElement *fe1 = new FieldElement( vec_0_0_0, vec_1_0_0);

	second.push_back( fe1 );

	Field::Correspondence corr;

	try {
		find_correspondences( first, second, corr );
        FAIL() << "Expected std::invalid_argument";
	} catch( invalid_argument const & err) {
        EXPECT_EQ( err.what(), string( "find_correspondences expects first vector to have non-zero size") );
	} catch( ... ) {
        FAIL() << "Expected std::invalid_argument";
	}
}

TEST_F(TestField, findCorrespondenceWithZeroSecondVectorShouldThrow) { 
	using namespace std;
	using namespace animesh;

	vector<FieldElement * > first;
	vector<FieldElement * > second;

	FieldElement *fe1 = new FieldElement( vec_0_0_0, vec_1_0_0);

	first.push_back( fe1 );

	Field::Correspondence corr;

	try {
		find_correspondences( first, second, corr );
        FAIL() << "Expected std::invalid_argument";
	} catch( invalid_argument const & err) {
        EXPECT_EQ( err.what(), string( "find_correspondences expects second vector to have non-zero size") );
	} catch( ... ) {
        FAIL() << "Expected std::invalid_argument";
	}
}


/* ******************************************************************************************
 * **
 * **  Test temporal transform
 * **
 * ******************************************************************************************/
TEST_F(TestField, TemporalTransform) {

}

// TEST_F(TestField, GetByIndexReturnsCorrectLocation) { 
//     Graph graph{ new NNEdgeManager{ 3} };
//     graph.addElement( el_1_1_1 );
//     graph.addElement( el_1_1_2 );
//     graph.addElement( el_1_1_3 );

//     Field field{ &graph };

//     std::tuple<Eigen::Vector3f, Eigen::Vector3f, Eigen::Vector3f> tuple = field.dataForGraphNode( 1 );

//     EXPECT_EQ( el_1_1_2.location(), std::get<0>(tuple) );
// }

// TEST_F(TestField, GetByIndexReturnsCorrectNormal) { 
//     Graph graph{ new NNEdgeManager{ 3} };
//     graph.addElement( el_1_1_1 );
//     graph.addElement( el_1_1_2 );
//     graph.addElement( el_1_1_3 );

//     Field field{ &graph };

//     std::tuple<Eigen::Vector3f, Eigen::Vector3f, Eigen::Vector3f> tuple = field.dataForGraphNode( 1 );

//     EXPECT_EQ( el_1_1_2.normal(), std::get<1>(tuple) );
// }

// TEST_F(TestField, GetByIndexReturnsCorrectTangent) { 
//     Graph graph{ new NNEdgeManager{ 3} };
//     graph.addElement( el_1_1_1 );
//     graph.addElement( el_1_1_2 );
//     graph.addElement( el_1_1_3 );

//     Field field{ &graph };

//     std::tuple<Eigen::Vector3f, Eigen::Vector3f, Eigen::Vector3f> tuple = field.dataForGraphNode( 1 );

//     Eigen::Vector3f tangent = std::get<2>(tuple);

//     EXPECT_FLOAT_EQ( 0.0f, el_1_1_2.normal().dot( tangent ) );
// }





// TEST_F( TestField, TangentsShouldBeUnitLengthAfterSmooth ) {
//     using namespace Eigen; 

//     EdgeManager *em = new GridEdgeManager{1.0f};
//     Graph *g = new Graph( em );

//     for( int y = 0; y<5; y++ ) {
//         for( int x = 0; x< 5; x++ ) {

//             Vector3f location { x, y, 0.0f };
//             Vector3f normal{ 0.0f, 0.0f, 1.0f };
//             Element *e = new Element( location, normal );

//             g->addElement( *e );
//         }
//     }

//     Field field{ g };

//     GraphNode * gn = *(g->begin());
//     Vector3f tangent = field.get_smoothed_tangent_data_for_node( gn );
//     EXPECT_FLOAT_EQ( 1.0f, tangent.norm() );
// }


// // Test that reprojection works
// TEST_F( TestField, ProjectionOfVectorParallelToNormalShouldBeZero) {
//     Eigen::Vector3f vector{ 1, 0, 0 };
//     Eigen::Vector3f normal{ 1, 0, 0 };

//     Eigen::Vector3f actual = reproject_to_tangent_space( vector, normal );

//     EXPECT_FLOAT_EQ( 0.0f, actual[0]);
//     EXPECT_FLOAT_EQ( 0.0f, actual[1]);
//     EXPECT_FLOAT_EQ( 0.0f, actual[2]);
// }

// TEST_F( TestField, ProjectionOfVectorPerpendicularToNormalShouldBeVector) {
//     Eigen::Vector3f vector{ 0, 1, 0 };
//     Eigen::Vector3f normal{ 1, 0, 0 };

//     Eigen::Vector3f actual = reproject_to_tangent_space( vector, normal );

//     EXPECT_FLOAT_EQ( vector[0], actual[0]);
//     EXPECT_FLOAT_EQ( vector[1], actual[1]);
//     EXPECT_FLOAT_EQ( vector[2], actual[2]);
// }

// TEST_F( TestField, ProjectionOfVectorShouldHaveUnitLength) {
//     Eigen::Vector3f vector{ 0, 17, 0 };
//     Eigen::Vector3f normal{ 1, 0, 0 };

//     Eigen::Vector3f actual = reproject_to_tangent_space( vector, normal );

//     EXPECT_FLOAT_EQ( 1.0f, actual.norm());
// }


// // TEST_F( TestField, SmoothFieldShouldGIveGoodValues ) {
// //     using namespace Eigen; 

// //     EdgeManager *em = new GridEdgeManager{1.0f};
// //     Graph *g = new Graph( em );

// //     for( int x = 0; x< 5; x++ ) {
// //         Vector3f location { x, 0.0f, 0.0f };
// //         Vector3f normal{ 0.0f, 0.0f, 1.0f };
// //         Element *e = new Element( location, normal );

// //         g->addElement( *e );
// //     }

// //     Field field{ g };

// //     // Manually set tangents in field
// //     field.dataForGraphNode( 0 )->set_tangent( Vector3f{ 0.0f, 1.0f, 0.0f } );
// //     field.dataForGraphNode( 1 )->set_tangent( Vector3f{ 1.0f, 0.0f, 0.0f } );
// //     field.dataForGraphNode( 2 )->set_tangent( Vector3f{ 0.0f, 1.0f, 0.0f } );

// //     field.smooth_once( );

// //     EXPECT_FLOAT_EQ( 1.0f, tangent.norm() );
// // }
