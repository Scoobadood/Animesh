#include <Graph/Graph.h>

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "TestGraph.h"
void TestGraph::SetUp( ){
    using namespace animesh;

    gn1 = new Graph<std::string, std::string>::GraphNode( "a" );
    gn2 = new Graph<std::string, std::string>::GraphNode( "b" );
}

void TestGraph::TearDown( ) {}


/* **********************************************************************
 * *                                                                    *
 * * Graph Constructor tests                                            *
 * *                                                                    *
 * **********************************************************************/

TEST_F(TestGraph, AddNodeByDataShouldAddNode) { 
    graph.add_node( "a" );

    EXPECT_EQ( graph.num_nodes(), 1 );
}

TEST_F(TestGraph, AddNodeDirectShouldAddNode) { 
    graph.add_node( gn1 );

    EXPECT_EQ( graph.num_nodes(), 1 );
}

TEST_F(TestGraph, AddNodeTwiceShouldAddTwoNodes) { 
    graph.add_node( gn1 );
    graph.add_node( gn2 );

    EXPECT_EQ( graph.num_nodes(), 2 );
}

TEST_F(TestGraph, AddEdgeWithNullFirstNodeShouldThrow) { 
    try {
        graph.add_edge( nullptr, gn1, 1.0, "edge data" );
        FAIL() << "Expected std::invalid_argument";
    }
    catch ( std::invalid_argument const & err ) {
        EXPECT_EQ( err.what(), std::string( "from node may not be null") );
    }
    catch ( ... ) {
        FAIL( ) << "Expected std::invalid_argument";
    }
}

TEST_F(TestGraph, AddEdgeWithNullSecondNodeShouldThrow) { 
    try {
        graph.add_edge( gn1, nullptr, 1.0, "edge data" );
        FAIL() << "Expected std::invalid_argument";
    }
    catch ( std::invalid_argument const & err ) {
        EXPECT_EQ( err.what(), std::string( "to node may not be null") );
    }
    catch ( ... ) {
        FAIL( ) << "Expected std::invalid_argument";
    }
}

TEST_F(TestGraph, AddEdgeWithNegativeWeightShouldThrow) { 
    graph.add_node( gn1 );
    graph.add_node( gn2 );

    try {
        graph.add_edge( gn1, gn2, -1.0, "edge data" );
        FAIL() << "Expected std::invalid_argument";
    }
    catch ( std::invalid_argument const & err ) {
        EXPECT_EQ( err.what(), std::string( "weight must be positive") );
    }
    catch ( ... ) {
        FAIL( ) << "Expected std::invalid_argument";
    }
}

TEST_F(TestGraph, AddEdgeWithUnknownFirstNodeShouldThrow) { 
    using namespace animesh; 

    graph.add_node( gn2 );

    Graph<std::string, std::string>::GraphNode * gn_unknown = new animesh::Graph<std::string, std::string>::GraphNode( "x" );

    try {
        graph.add_edge( gn_unknown, gn2, 1.0, "edge data" );
        FAIL() << "Expected std::invalid_argument";
    }
    catch ( std::invalid_argument const & err ) {
        EXPECT_EQ( err.what(), std::string( "from node is unknown") );
    }
    catch ( ... ) {
        FAIL( ) << "Expected std::invalid_argument";
    }
}

TEST_F(TestGraph, AddEdgeWithUnknownSecondNodeShouldThrow) { 
    using namespace animesh; 

    graph.add_node( gn1 );

    Graph<std::string, std::string>::GraphNode * gn_unknown = new animesh::Graph<std::string, std::string>::GraphNode( "x" );

    try {
        graph.add_edge( gn1, gn_unknown, 1.0, "edge data" );
        FAIL() << "Expected std::invalid_argument";
    }
    catch ( std::invalid_argument const & err ) {
        EXPECT_EQ( err.what(), std::string( "to node is unknown") );
    }
    catch ( ... ) {
        FAIL( ) << "Expected std::invalid_argument";
    }
}

TEST_F(TestGraph, AddEdgeIncreasesEdgeCount) { 
    using namespace animesh; 

    graph.add_node( gn1 );
    graph.add_node( gn2 );

    size_t before_count = graph.num_edges( );

    graph.add_edge( gn1, gn2, 1.0, "edge data" );

    size_t after_count = graph.num_edges( );
    EXPECT_EQ( before_count + 1, after_count );
}



TEST_F(TestGraph, AddGraphNodeIncreasesNodeCount) { 
    size_t before_count = graph.num_nodes( );

    graph.add_node( gn1 );

    size_t after_count = graph.num_nodes( );
    EXPECT_EQ( before_count + 1, after_count );
}

TEST_F(TestGraph, AddDuplicateGraphNodeShouldThrow) { 
    graph.add_node( gn1 );

    try {
        graph.add_node( gn1 );
        FAIL() << "Expected std::invalid_argument";
    }
    catch ( std::invalid_argument const & err ) {
        EXPECT_EQ( err.what(), std::string( "can't add node to graph when it's already there") );
    }
    catch ( ... ) {
        FAIL( ) << "Expected std::invalid_argument";
    }
}

TEST_F(TestGraph, AddDuplicateEdgeShouldThrow) { 
    graph.add_node( gn1 );
    graph.add_node( gn2 );

    graph.add_edge( gn1, gn2, 1.0, "edge data" );

    try {
        graph.add_edge( gn1, gn2, 1.0, "edge data" );
        FAIL() << "Expected std::invalid_argument";
    }
    catch ( std::invalid_argument const & err ) {
        EXPECT_EQ( err.what(), std::string( "can't insert duplicate edge") );
    }
    catch ( ... ) {
        FAIL( ) << "Expected std::invalid_argument";
    }
}

TEST_F(TestGraph, AddReverseEdgeShouldNotThrow) { 
    graph.add_node( gn1 );
    graph.add_node( gn2 );
    graph.add_edge( gn1, gn2, 1.0, "edge data" );
    graph.add_edge( gn2, gn1, 1.0, "edge data" );
    EXPECT_EQ( 2, graph.num_edges() );
}

TEST_F(TestGraph, UnlinkedNodesHaveNoNeighbours ) { 
    graph.add_node( gn1 );
    graph.add_node( gn2 );

    EXPECT_EQ( 0, graph.neighbours( gn1 ).size() );
    EXPECT_EQ( 0, graph.neighbours( gn2 ).size() );
}

TEST_F(TestGraph, ToNodeOfEdgeIsNeighbourOfFromNode ) { 
    graph.add_node( gn1 );
    graph.add_node( gn2 );
    graph.add_edge( gn1, gn2, 1.0, "edge data" );

    EXPECT_EQ( 1, graph.neighbours( gn1 ).size() );
    EXPECT_EQ( gn2, graph.neighbours( gn1 )[0] );
}

TEST_F(TestGraph, FromNodeOfEdgeIsNotNeighbourOfToNode ) { 
    graph.add_node( gn1 );
    graph.add_node( gn2 );

    graph.add_edge( gn1, gn2, 1.0, "edge data" );

    EXPECT_EQ( 0, graph.neighbours( gn2 ).size() );
}