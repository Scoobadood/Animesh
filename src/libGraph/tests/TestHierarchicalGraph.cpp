#include <Graph/HierarchicalGraph.h>

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "TestHierarchicalGraph.h"

std::string merge_strings( const std::string& s1, const std::string& s2 ) {
    return s1+s2;
}

std::string propagate_strings( std::string s1, std::string s2 ){
    return s1;
}

void TestHierarchicalGraph::SetUp( ){
    using namespace animesh;

    gn1 = new Graph<std::string, std::string>::GraphNode( "a" );
    gn2 = new Graph<std::string, std::string>::GraphNode( "b" );
}
void TestHierarchicalGraph::TearDown( ) {}


/* **********************************************************************
 * *                                                                    *
 * * Graph Constructor tests                                            *
 * *                                                                    *
 * **********************************************************************/

TEST_F(TestHierarchicalGraph, AddNodeByDataShouldAddNode) { 
    graph.add_node( "a" );

    EXPECT_EQ( graph.num_nodes(), 1 );
}

TEST_F(TestHierarchicalGraph, AddNodeDirectShouldAddNode) { 
    graph.add_node( gn1 );

    EXPECT_EQ( graph.num_nodes(), 1 );
}

TEST_F(TestHierarchicalGraph, AddNodeTwiceShouldAddTwoNodes) { 
    graph.add_node( gn1 );
    graph.add_node( gn2 );

    EXPECT_EQ( graph.num_nodes(), 2 );
}

TEST_F(TestHierarchicalGraph, AddEdgeWithNullFirstNodeShouldThrow) { 
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

TEST_F(TestHierarchicalGraph, AddEdgeWithNullSecondNodeShouldThrow) { 
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

TEST_F(TestHierarchicalGraph, ConstructGraphNodeWithNullFirstNodeShouldThrow) { 
    using namespace animesh;

    try {
        Graph<std::string, std::string>::GraphNode * gn = new Graph<std::string, std::string>::GraphNode( nullptr, gn1, merge_strings );
        FAIL() << "Expected std::invalid_argument";
    }
    catch ( std::invalid_argument const & err ) {
        EXPECT_EQ( err.what(), std::string( "first node may not be null") );
    }
    catch ( ... ) {
        FAIL( ) << "Expected std::invalid_argument";
    }
}

TEST_F(TestHierarchicalGraph, ConstructGraphNodeWithNullSecondNodeShouldThrow) { 
    using namespace animesh;

    try {
        Graph<std::string, std::string>::GraphNode * gn = new Graph<std::string, std::string>::GraphNode( gn1, nullptr, merge_strings );
        FAIL() << "Expected std::invalid_argument";
    }
    catch ( std::invalid_argument const & err ) {
        EXPECT_EQ( err.what(), std::string( "second node may not be null") );
    }
    catch ( ... ) {
        FAIL( ) << "Expected std::invalid_argument";
    }
}

TEST_F(TestHierarchicalGraph, ConstructGraphNodeWithNullSoleNodeShouldThrow) { 
    using namespace animesh;

    try {
        Graph<std::string, std::string>::GraphNode * gn = new Graph<std::string, std::string>::GraphNode( nullptr);
        FAIL() << "Expected std::invalid_argument";
    }
    catch ( std::invalid_argument const & err ) {
        EXPECT_EQ( err.what(), std::string( "node may not be null") );
    }
    catch ( ... ) {
        FAIL( ) << "Expected std::invalid_argument";
    }
}

TEST_F(TestHierarchicalGraph, AddEdgeWithNegativeWeightShouldThrow) { 
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

TEST_F(TestHierarchicalGraph, AddEdgeWithUnknownFirstNodeShouldThrow) { 
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

TEST_F(TestHierarchicalGraph, AddEdgeWithUnknownSecondNodeShouldThrow) { 
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

TEST_F(TestHierarchicalGraph, AddEdgeIncreasesEdgeCount) { 
    using namespace animesh; 

    graph.add_node( gn1 );
    graph.add_node( gn2 );

    size_t before_count = graph.num_edges( );

    graph.add_edge( gn1, gn2, 1.0, "edge data" );

    size_t after_count = graph.num_edges( );
    EXPECT_EQ( before_count + 1, after_count );
}

TEST_F(TestHierarchicalGraph, MakeMergedNodeCallsMergeFunction) { 
    using namespace animesh;

    Graph<std::string, std::string>::GraphNode * gn3 = new Graph<std::string, std::string>::GraphNode( gn1, gn2, merge_strings);
    EXPECT_EQ( "ab", gn3->data() );

}
TEST_F(TestHierarchicalGraph, MakeMergedNodeCallsSetsParentOfFirstNode) { 
    using namespace animesh;

    Graph<std::string, std::string>::GraphNode * gn3 = new Graph<std::string, std::string>::GraphNode( gn1, gn2, merge_strings);
    EXPECT_EQ( gn3, gn1->parent() );
}

TEST_F(TestHierarchicalGraph, MakeMergedNodeCallsSetsParentOfSecondNode) { 
    using namespace animesh;

    Graph<std::string, std::string>::GraphNode * gn3 = new Graph<std::string, std::string>::GraphNode( gn1, gn2, merge_strings);
    EXPECT_EQ( gn3, gn2->parent() );
}

TEST_F(TestHierarchicalGraph, MakeMergedNodeCallsSetsChildren) { 
    using namespace animesh;

    Graph<std::string, std::string>::GraphNode * gn3 = new Graph<std::string, std::string>::GraphNode( gn1, gn2, merge_strings);
    EXPECT_EQ( 2, gn3->children().size() );
}

TEST_F(TestHierarchicalGraph, MakeMergedNodeCallsSetsChild1ToFirstNode) { 
    using namespace animesh;

    Graph<std::string, std::string>::GraphNode * gn3 = new Graph<std::string, std::string>::GraphNode( gn1, gn2, merge_strings);
    EXPECT_EQ( gn1, gn3->children()[0] );
}

TEST_F(TestHierarchicalGraph, MakeMergedNodeCallsSetsChild2ToSecondNode) { 
    using namespace animesh;

    Graph<std::string, std::string>::GraphNode * gn3 = new Graph<std::string, std::string>::GraphNode( gn1, gn2, merge_strings);
    EXPECT_EQ( gn2, gn3->children()[1] );
}

TEST_F(TestHierarchicalGraph, AddGraphNodeIncreasesNodeCount) { 
    size_t before_count = graph.num_nodes( );

    graph.add_node( gn1 );

    size_t after_count = graph.num_nodes( );
    EXPECT_EQ( before_count + 1, after_count );
}

TEST_F(TestHierarchicalGraph, AddDuplicateGraphNodeShouldThrow) { 
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

TEST_F(TestHierarchicalGraph, AddDuplicateEdgeShouldThrow) { 
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

TEST_F(TestHierarchicalGraph, AddReverseEdgeShouldNotThrow) { 
    graph.add_node( gn1 );
    graph.add_node( gn2 );
    graph.add_edge( gn1, gn2, 1.0, "edge data" );
    graph.add_edge( gn2, gn1, 1.0, "edge data" );
    EXPECT_EQ( 2, graph.num_edges() );
}

TEST_F(TestHierarchicalGraph, UnlinkedNodesHaveNoNeighbours ) { 
    graph.add_node( gn1 );
    graph.add_node( gn2 );

    EXPECT_EQ( 0, graph.neighbours( gn1 ).size() );
    EXPECT_EQ( 0, graph.neighbours( gn2 ).size() );
}

TEST_F(TestHierarchicalGraph, ToNodeOfEdgeIsNeighbourOfFromNode ) { 
    graph.add_node( gn1 );
    graph.add_node( gn2 );
    graph.add_edge( gn1, gn2, 1.0, "edge data" );

    EXPECT_EQ( 1, graph.neighbours( gn1 ).size() );
    EXPECT_EQ( gn2, graph.neighbours( gn1 )[0] );
}

TEST_F(TestHierarchicalGraph, FromNodeOfEdgeIsNotNeighbourOfToNode ) { 
    graph.add_node( gn1 );
    graph.add_node( gn2 );

    graph.add_edge( gn1, gn2, 1.0, "edge data" );

    EXPECT_EQ( 0, graph.neighbours( gn2 ).size() );
}

TEST_F(TestHierarchicalGraph, SimplifyTwoNodeGraphIsNotNull ) { 
    using namespace animesh;

    graph.add_node( gn1 );
    graph.add_node( gn2 );
    graph.add_edge( gn1, gn2, 1., "" );

    Graph<std::string, std::string> * new_graph = graph.simplify( );

    EXPECT_NE( nullptr, new_graph );
}


TEST_F(TestHierarchicalGraph, SimplifyTwoNodeGraphGivesOneNodeGraph ) { 
    using namespace animesh;

    graph.add_node( gn1 );
    graph.add_node( gn2 );
    graph.add_edge( gn1, gn2, 1., "" );

    Graph<std::string, std::string> * new_graph = graph.simplify( );

    EXPECT_EQ( 1, new_graph->num_nodes() );
}


TEST_F(TestHierarchicalGraph, SimplifyGraphWithNoEdgesShouldDoNothing) { 
    using namespace animesh;

    graph.add_node( gn1 );
    graph.add_node( gn2 );

    Graph<std::string, std::string> * new_graph = graph.simplify( );

    EXPECT_EQ( nullptr, new_graph );
}

