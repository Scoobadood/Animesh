#include "TestField.h"

#include <Graph/Graph.h>
#include <Graph/GridGraphBuilder.h>
#include <Field/Field.h>


void TestField::SetUp( ) {}
void TestField::TearDown( ) {}

TEST_F(TestField, SizeAccuatelyReportsSize) { 
	std::vector<Element> elements;
	elements.push_back( el_1_1_1 );
	elements.push_back( el_1_1_2 );
	elements.push_back( el_1_1_3 );

	GraphBuilder<void *> * gb = new GridGraphBuilder<void *>( 1.0f );
	Field * field = new Field( gb, elements );

	// Hard set tangents
	const std::vector<const FieldElement *> fes = field->elements( );
	const_cast<FieldElement *>(fes[0])->m_tangent = Eigen::Vector3f{ -1.50f, 0.0f, 1.0f };
	const_cast<FieldElement *>(fes[1])->m_tangent = Eigen::Vector3f{  0.0f, 0.0f, 1.0f };
	const_cast<FieldElement *>(fes[2])->m_tangent = Eigen::Vector3f{  1.50f, 0.0f, 1.0f };

	std::cout << "tan{1} = [" << fes[0]->m_tangent[0] << ", " <<  fes[0]->m_tangent[1] << ", " << fes[0]->m_tangent[2] << ";";
	std::cout << fes[1]->m_tangent[0] << ", " <<  fes[1]->m_tangent[1] << ", " << fes[1]->m_tangent[2] << ";";
	std::cout << fes[2]->m_tangent[0] << ", " <<  fes[2]->m_tangent[1] << ", " << fes[2]->m_tangent[2] << "]';" << std::endl;

	for( int i=0; i<3; i++ ) {
		field->smooth( );
		std::cout << "tan{"<<i+2<<"} = [" << fes[0]->m_tangent[0] << ", " <<  fes[0]->m_tangent[1] << ", " << fes[0]->m_tangent[2] << ";";
		std::cout << fes[1]->m_tangent[0] << ", " <<  fes[1]->m_tangent[1] << ", " << fes[1]->m_tangent[2] << ";";
		std::cout << fes[2]->m_tangent[0] << ", " <<  fes[2]->m_tangent[1] << ", " << fes[2]->m_tangent[2] << "]';" << std::endl;
	}
}


// TEST_F(TestField, ShouldThrowWhenRequestingOutOfRangeItem) { 
//     Graph graph{ new NNEdgeManager{ 3} };
//     graph.addElement( el_1_1_1 );
//     graph.addElement( el_1_1_2 );
//     graph.addElement( el_1_1_3 );

//     Field field{ &graph };

//  	try {
//         field.dataForGraphNode( 4 );
//         FAIL() << "Expected std::invalid_argument";
//     }
//     catch ( std::invalid_argument const & err ) {
//         EXPECT_EQ( err.what(), std::string( "Index out of range") );
//     }
//     catch ( ... ) {
//         FAIL( ) << "Expected std::invalid_argument";
//     }
// }

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
