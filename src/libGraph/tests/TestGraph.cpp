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
  EXPECT_DEATH(
        graph.add_edge( nullptr, gn1, 1.0, "edge data" ),
        "from_node != nullptr");
}

TEST_F(TestGraph, AddEdgeWithNullSecondNodeShouldThrow) {
  EXPECT_DEATH(
        graph.add_edge( gn1, nullptr, 1.0, "edge data" ),
        "to_node != nullptr");
}

TEST_F(TestGraph, AddEdgeWithNegativeWeightShouldThrow) {
    graph.add_node( gn1 );
    graph.add_node( gn2 );

    EXPECT_DEATH(
        graph.add_edge( gn1, gn2, -1.0, "edge data" ),
        "weight >= 0");
}

TEST_F(TestGraph, AddEdgeWithUnknownFirstNodeShouldThrow) {
    using namespace animesh;

    graph.add_node( gn2 );

    Graph<std::string, std::string>::GraphNode * gn_unknown = new animesh::Graph<std::string, std::string>::GraphNode( "x" );

    EXPECT_DEATH(
        graph.add_edge( gn_unknown, gn2, 1.0, "edge data" ),
        "Assertion failed: .*add_edge.*");
}

TEST_F(TestGraph, AddEdgeWithUnknownSecondNodeShouldThrow) {
    using namespace animesh;

    graph.add_node( gn1 );

    Graph<std::string, std::string>::GraphNode * gn_unknown = new animesh::Graph<std::string, std::string>::GraphNode( "x" );

    EXPECT_DEATH(
        graph.add_edge( gn1, gn_unknown, 1.0, "edge data" ),
        "Assertion failed: .*function add_edge.*");
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

    EXPECT_DEATH(
        graph.add_node( gn1 ),
        "Assert failed: .*add_node.*");
}

TEST_F(TestGraph, AddDuplicateEdgeShouldThrow) {
    graph.add_node( gn1 );
    graph.add_node( gn2 );

    graph.add_edge( gn1, gn2, 1.0, "edge data" );

    EXPECT_DEATH(
        graph.add_edge( gn1, gn2, 1.0, "edge data" ),
        "");
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
