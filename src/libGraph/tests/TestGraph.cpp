#include <string>
#include <vector>

#include <Graph/Graph.h>

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "TestGraph.h"
#include <spdlog/spdlog.h>

#define EXPECT_THROW_WITH_MESSAGE(stmt, etype, whatstring) EXPECT_THROW( \
        try { \
            stmt; \
        } catch (const etype& ex) { \
            EXPECT_EQ(std::string(ex.what()), whatstring); \
            throw; \
        } \
    , etype)

void TestGraph::SetUp( ){
    using namespace animesh;

    gn1 = new Graph<std::string, float>::GraphNode( "a" );
    gn2 = new Graph<std::string, float>::GraphNode( "b" );
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

TEST_F(TestGraph, AddNodeByDataTwiceShouldIncreaseNodeCount) {
    graph.add_node( "a" );
    graph.add_node( "a" );
    EXPECT_EQ( graph.num_nodes(), 2 );
}

TEST_F(TestGraph, AddDifferentNodesByDataShouldIncreaseNodeCount) {
    graph.add_node( "a" );
    graph.add_node( "b" );
    EXPECT_EQ( graph.num_nodes(), 2 );
}

TEST_F(TestGraph, AddNodesShouldIncreaseNodeCount) {
    auto n = animesh::Graph<std::string, float>::make_node("a");
    graph.add_node( n );
    EXPECT_EQ( graph.num_nodes(), 1 );
}

TEST_F(TestGraph, AddDuplicateNodesShouldNotIncreaseNodeCount) {
    auto n = animesh::Graph<std::string, float>::make_node("a");
    graph.add_node( n );
    graph.add_node( n );
    EXPECT_EQ( graph.num_nodes(), 1 );
}

TEST_F(TestGraph, AddShouldIncreaseEdgeCount) {
    auto n1 = graph.add_node( "a" );
    auto n2 = graph.add_node( "b" );
    graph.add_edge(n1, n2, 1.0f);
    EXPECT_EQ( graph.num_edges(), 1 );
}

TEST_F(TestGraph, AddDuplicateEdgeShouldNotIncreaseEdgeCount) {

    auto n1 = graph.add_node( "a" );
    auto n2 = graph.add_node( "b" );
    graph.add_edge( n1, n2, 1.0 );
    size_t before_count = graph.num_edges( );

    graph.add_edge( n1, n2, 1.0 );
    size_t after_count = graph.num_edges( );

    EXPECT_EQ( before_count, after_count );
}

TEST_F(TestGraph, AddReverseEdgeShouldIncreaseCountInDirectedGraph) {

    auto n1 = graph.add_node( "a" );
    auto n2 = graph.add_node( "b" );
    graph.add_edge( n1, n2, 1.0 );
    size_t before_count = graph.num_edges( );

    graph.add_edge( n2, n1, 1.0 );
    size_t after_count = graph.num_edges( );

    EXPECT_EQ( before_count + 1, after_count );
}

TEST_F(TestGraph, AddReverseEdgeShouldNotIncreaseCountInUndirectedGraph) {

    auto n1 = undirected_graph.add_node( "a" );
    auto n2 = undirected_graph.add_node( "b" );
    undirected_graph.add_edge( n1, n2, 1.0 );
    size_t before_count = graph.num_edges( );

    undirected_graph.add_edge( n2, n1, 1.0 );
    size_t after_count = graph.num_edges( );

    EXPECT_EQ( before_count, after_count );
}

TEST_F(TestGraph, UnlinkedNodesHaveNoNeighbours ) {
    auto n1 = graph.add_node( "a" );
    auto n2 = graph.add_node( "b" );

    EXPECT_EQ( 0, graph.neighbours( n1 ).size());
    EXPECT_EQ( 0, graph.neighbours( n2 ).size() );
}

TEST_F(TestGraph, ToNodeOfEdgeIsNeighbourOfFromNode ) {
    auto from = graph.add_node( "a" );
    auto to = graph.add_node( "b" );
    graph.add_edge( from, to, 1.0 );

    auto nbr = graph.neighbours( from );
    EXPECT_EQ( 1, nbr.size() );
    EXPECT_EQ( to, nbr[0] );
}

TEST_F(TestGraph, FromNodeOfEdgeIsNotNeighbourOfToNode ) {
    auto from = graph.add_node( "a" );
    auto to = graph.add_node( "b" );
    graph.add_edge( from, to, 1.0 );

    auto nbr = graph.neighbours( to );
    EXPECT_EQ( 0, nbr.size() );
}

TEST_F(TestGraph, FromNodeOfEdgeIsNeighbourOfToNodeInUndirectedGraph ) {
    auto from = undirected_graph.add_node( "a" );
    auto to = undirected_graph.add_node( "b" );
    undirected_graph.add_edge( from, to, 1.0 );

    auto nbr = undirected_graph.neighbours( to );
    EXPECT_EQ( 1, nbr.size() );
    EXPECT_EQ( from, nbr[0] );
}

TEST_F(TestGraph, RemoveNodeAlsoRemovesEdges ) {
    auto from = graph.add_node( "a" );
    auto to = graph.add_node( "b" );
    graph.add_edge( from, to, 1.0 );
    EXPECT_EQ(1, graph.num_edges());
    EXPECT_TRUE(graph.has_edge(from, to));

    graph.remove_node(from);
    EXPECT_EQ(0, graph.num_edges());
}

TEST_F(TestGraph, RemoveNodeOnlyRemovesIncidentEdges ) {
    auto a = undirected_graph.add_node( "a" );
    auto b = undirected_graph.add_node( "b" );
    auto c = undirected_graph.add_node( "c" );
    undirected_graph.add_edge( a, b, 1.0 );
    undirected_graph.add_edge( a, c, 1.0 );
    undirected_graph.add_edge( b, c, 1.0 );

    EXPECT_EQ(3, undirected_graph.num_edges());
    EXPECT_TRUE(undirected_graph.has_edge(a, b));
    EXPECT_TRUE(undirected_graph.has_edge(b, a));
    EXPECT_TRUE(undirected_graph.has_edge(a, c));
    EXPECT_TRUE(undirected_graph.has_edge(c, a));
    EXPECT_TRUE(undirected_graph.has_edge(b, c));
    EXPECT_TRUE(undirected_graph.has_edge(c, b));

    undirected_graph.remove_node(b);
    EXPECT_EQ(1, undirected_graph.num_edges());
    EXPECT_TRUE(undirected_graph.has_edge(a, c));
    EXPECT_TRUE(undirected_graph.has_edge(c, a));
}

TEST_F(TestGraph, GraphAssignmentWorks ) {
    auto g2 = undirected_graph;
}
