#include "TestGraphSimplifier.h"
#include "Graph/GraphSimplifier.h"
#include "Graph/Graph.h"

using GraphSimplifier = animesh::GraphSimplifier<std::string, float>;
using GraphMapping = animesh::GraphSimplifier<std::string, float>::GraphMapping;
using GraphNode = animesh::Graph<std::string, float>::GraphNode;
using Graph = animesh::Graph<std::string, float>;

std::string merge_strings( const std::string& s1, const std::string& s2 ) {
    return s1+s2;
}

std::string propagate_strings( const std::string& parent_data, const std::string& child_data ){
    return parent_data;
}

void TestGraphSimplifier::SetUp( ){
    using namespace animesh;
}

void TestGraphSimplifier::TearDown( ) {}

TEST_F(TestGraphSimplifier, SimplifyTwoNodeGraphIsNotNull ) { 
    auto from = graph.add_node( "a" );
    auto to = graph.add_node( "b" );
    graph.add_edge( from, to, 1.0f );

    GraphSimplifier s{ merge_strings, propagate_strings };
    std::pair<Graph *, GraphMapping> pair = s.simplify( &graph );

    EXPECT_NE( nullptr, pair.first );
}

TEST_F(TestGraphSimplifier, SimplifyTwoNodeGraphNodeHasOneNode ) {
    auto from = graph.add_node( "a" );
    auto to = graph.add_node( "b" );
    graph.add_edge( from, to, 1.0f );

    GraphSimplifier s{ merge_strings, propagate_strings };
    auto pair = s.simplify( &graph );
    auto new_graph = pair.first;

    EXPECT_EQ( 1, new_graph->num_nodes());
}

TEST_F(TestGraphSimplifier, SimplifyTwoNodeGraphNodeIsMergedCorrectly ) {
    auto from = graph.add_node( "a" );
    auto to = graph.add_node( "b" );
    graph.add_edge( from, to, 1.0f );

    GraphSimplifier s{ merge_strings, propagate_strings };
    auto pair = s.simplify( &graph );
    auto new_graph = pair.first;

    const auto new_node_data = new_graph->node_data()[0];
    EXPECT_TRUE( (new_node_data == "ab" ) || (new_node_data == "ba") );
}

TEST_F(TestGraphSimplifier, SimplifyTwoNodeGraphMappingPropagatesCorrectly ) {
    auto from = graph.add_node( "a" );
    auto to = graph.add_node( "b" );
    graph.add_edge( from, to, 1.0f );

    GraphSimplifier s{ merge_strings, propagate_strings };
    std::pair<Graph *, GraphMapping> pair = s.simplify( &graph );
    GraphMapping gm = pair.second;

    auto new_node_data = pair.first->node_data();
    auto new_nodes = pair.first->nodes();
    EXPECT_EQ(1, new_nodes.size());

    EXPECT_EQ( new_nodes.at(0), gm.parent(from));
    EXPECT_EQ( new_nodes.at(0), gm.parent(to));
    EXPECT_TRUE( new_node_data.at(0) == "ab" || new_node_data.at(0) == "ba");

    gm.propagate( );

    EXPECT_TRUE( from->data() == "ab" || from->data() == "ba");
    EXPECT_TRUE( to->data() == "ab" || to->data() == "ba");
}


TEST_F(TestGraphSimplifier, SimplifyGraphWithNoEdgesShouldThrow) {
    auto from = graph.add_node( "a" );
    auto to = graph.add_node( "b" );

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