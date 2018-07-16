#include "TestGraphSimplifier.h"
#include "Graph/GraphSimplifier.h"
#include "Graph/Graph.h"

using GraphSimplifier = animesh::GraphSimplifier<std::string, std::string>;
using GraphMapping = animesh::GraphSimplifier<std::string, std::string>::GraphMapping;
using GraphNode = animesh::Graph<std::string, std::string>::GraphNode;
using Graph = animesh::Graph<std::string, std::string>;

std::string merge_strings( const std::string& s1, const std::string& s2 ) {
    return s1+s2;
}

std::string propagate_strings( const std::string& parent_data, const std::string& child_data ){
    return parent_data;
}

void TestGraphSimplifier::SetUp( ){
    using namespace animesh;

    gn1 = new GraphNode( "a" );
    gn2 = new GraphNode( "b" );
}

void TestGraphSimplifier::TearDown( ) {}

TEST_F(TestGraphSimplifier, SimplifyTwoNodeGraphIsNotNull ) { 
    graph.add_node( gn1 );
    graph.add_node( gn2 );
    graph.add_edge( gn1, gn2, 1., "" );

    GraphSimplifier s{ merge_strings, propagate_strings };
    std::pair<Graph *, GraphMapping> pair = s.simplify( &graph );

    EXPECT_NE( nullptr, pair.first );
}

TEST_F(TestGraphSimplifier, SimplifyTwoNodeGraphNodeHasOneNode ) { 
    graph.add_node( gn1 );
    graph.add_node( gn2 );
    graph.add_edge( gn1, gn2, 1., "" );

    GraphSimplifier s{ merge_strings, propagate_strings };
    std::pair<Graph *, GraphMapping> pair = s.simplify( &graph );
    Graph * new_graph = pair.first;

    EXPECT_EQ( 1, new_graph->num_nodes());
}

TEST_F(TestGraphSimplifier, SimplifyTwoNodeGraphNodeIsMergedCorrectly ) { 
    graph.add_node( gn1 );
    graph.add_node( gn2 );
    graph.add_edge( gn1, gn2, 1., "" );

    GraphSimplifier s{ merge_strings, propagate_strings };
    std::pair<Graph *, GraphMapping> pair = s.simplify( &graph );
    Graph * new_graph = pair.first;

    EXPECT_EQ( "ab", new_graph->nodes()[0]->data());
}

TEST_F(TestGraphSimplifier, SimplifyTwoNodeGraphMappingPropagatesCorrectly ) { 
    graph.add_node( gn1 );
    graph.add_node( gn2 );
    graph.add_edge( gn1, gn2, 1., "" );

    GraphSimplifier s{ merge_strings, propagate_strings };
    std::pair<Graph *, GraphMapping> pair = s.simplify( &graph );
    Graph * new_graph = pair.first;
    GraphMapping gm = pair.second;

    EXPECT_EQ( "a", graph.nodes()[0]->data());
    EXPECT_EQ( "b", graph.nodes()[1]->data());

    gm.propagate( );

    EXPECT_EQ( "ab", graph.nodes()[0]->data());
    EXPECT_EQ( "ab", graph.nodes()[1]->data());
}


TEST_F(TestGraphSimplifier, SimplifyGraphWithNoEdgesShouldThrow) { 
    graph.add_node( gn1 );
    graph.add_node( gn2 );

    GraphSimplifier s{ merge_strings, propagate_strings };

	try {
		s.simplify( &graph );
        FAIL() << "Expected std::invalid_argument";
    }
    catch ( std::invalid_argument const & err ) {
        EXPECT_EQ( err.what(), std::string( "Can't simplify Graph with no edges") );
    }
    catch ( ... ) {
        FAIL( ) << "Expected std::invalid_argument";
    }
}