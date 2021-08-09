#include <Graph/Graph.h>

#include "gtest/gtest.h"
#include "TestGraph.h"
#include <memory>

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

    graph = std::make_shared<Graph<std::string,float>>(true);
    undirected_graph = std::make_shared<Graph<std::string,float>>();

    gn1 = std::make_shared<GraphNode>( "a" );
    gn2 = std::make_shared<GraphNode>("b" );
}

void TestGraph::TearDown( ) {}


/* **********************************************************************
 * *                                                                    *
 * * Graph Constructor tests                                            *
 * *                                                                    *
 * **********************************************************************/

TEST_F(TestGraph, DGAddNodeByDataShouldAddNode) {
    graph->add_node( "a" );

    EXPECT_EQ( graph->num_nodes(), 1 );
}

TEST_F(TestGraph, UGAddNodeByDataShouldAddNode) {
    undirected_graph->add_node( "a" );

    EXPECT_EQ( undirected_graph->num_nodes(), 1 );
}

TEST_F(TestGraph, DGAddNodeByDataTwiceShouldIncreaseNodeCount) {
    graph->add_node( "a" );
    graph->add_node( "a" );
    EXPECT_EQ( graph->num_nodes(), 2 );
}

TEST_F(TestGraph, UGAddNodeByDataTwiceShouldIncreaseNodeCount) {
    undirected_graph->add_node( "a" );
    undirected_graph->add_node( "a" );
    EXPECT_EQ( undirected_graph->num_nodes(), 2 );
}

TEST_F(TestGraph, DGAddDifferentNodesByDataShouldIncreaseNodeCount) {
    graph->add_node( "a" );
    graph->add_node( "b" );
    EXPECT_EQ( graph->num_nodes(), 2 );
}

TEST_F(TestGraph, UGAddDifferentNodesByDataShouldIncreaseNodeCount) {
    undirected_graph->add_node( "a" );
    undirected_graph->add_node( "b" );
    EXPECT_EQ( undirected_graph->num_nodes(), 2 );
}

TEST_F(TestGraph, DGAddNodesShouldIncreaseNodeCount) {
    auto n = animesh::Graph<std::string, float>::make_node("a");
    graph->add_node( n );
    EXPECT_EQ( graph->num_nodes(), 1 );
}

TEST_F(TestGraph, UGAddNodesShouldIncreaseNodeCount) {
    auto n = animesh::Graph<std::string, float>::make_node("a");
    undirected_graph->add_node( n );
    EXPECT_EQ( undirected_graph->num_nodes(), 1 );
}

TEST_F(TestGraph, DGAddDuplicateNodesShouldNotIncreaseNodeCount) {
    auto n = animesh::Graph<std::string, float>::make_node("a");
    graph->add_node( n );
    graph->add_node( n );
    EXPECT_EQ( graph->num_nodes(), 1 );
}

TEST_F(TestGraph, UGAddDuplicateNodesShouldNotIncreaseNodeCount) {
    auto n = animesh::Graph<std::string, float>::make_node("a");
    undirected_graph->add_node( n );
    undirected_graph->add_node( n );
    EXPECT_EQ( undirected_graph->num_nodes(), 1 );
}

TEST_F(TestGraph, DGAddShouldIncreaseEdgeCount) {
    auto n1 = graph->add_node( "a" );
    auto n2 = graph->add_node( "b" );
    graph->add_edge(n1, n2, 1.0f);
    EXPECT_EQ( graph->num_edges(), 1 );
}

TEST_F(TestGraph, UGAddShouldIncreaseEdgeCountByTwo) {
    auto n1 = undirected_graph->add_node( "a" );
    auto n2 = undirected_graph->add_node( "b" );
    undirected_graph->add_edge(n1, n2, 1.0f);
    EXPECT_EQ( undirected_graph->num_edges(), 2 );
}

TEST_F(TestGraph, DGAddDuplicateEdgeShouldNotIncreaseEdgeCount) {

    auto n1 = graph->add_node( "a" );
    auto n2 = graph->add_node( "b" );
    graph->add_edge( n1, n2, 1.0 );
    size_t before_count = graph->num_edges( );

    graph->add_edge( n1, n2, 1.0 );
    size_t after_count = graph->num_edges( );

    EXPECT_EQ( before_count, after_count );
}

TEST_F(TestGraph, UGAddDuplicateEdgeShouldNotIncreaseEdgeCount) {

    auto n1 = undirected_graph->add_node( "a" );
    auto n2 = undirected_graph->add_node( "b" );
    undirected_graph->add_edge( n1, n2, 1.0 );
    size_t before_count = undirected_graph->num_edges( );

    undirected_graph->add_edge( n1, n2, 1.0 );
    size_t after_count = undirected_graph->num_edges( );

    EXPECT_EQ( before_count, after_count );
}

TEST_F(TestGraph, AddReverseEdgeShouldIncreaseCountInDirectedGraph) {

    auto n1 = graph->add_node( "a" );
    auto n2 = graph->add_node( "b" );
    graph->add_edge( n1, n2, 1.0 );
    size_t before_count = graph->num_edges( );

    graph->add_edge( n2, n1, 1.0 );
    size_t after_count = graph->num_edges( );

    EXPECT_EQ( before_count + 1, after_count );
}

TEST_F(TestGraph, AddReverseEdgeShouldNotIncreaseCountInUndirectedGraph) {

    auto n1 = undirected_graph->add_node( "a" );
    auto n2 = undirected_graph->add_node( "b" );
    undirected_graph->add_edge( n1, n2, 1.0 );
    size_t before_count = graph->num_edges( );

    undirected_graph->add_edge( n2, n1, 1.0 );
    size_t after_count = graph->num_edges( );

    EXPECT_EQ( before_count, after_count );
}

TEST_F(TestGraph, UnlinkedNodesHaveNoNeighbours ) {
    auto n1 = graph->add_node( "a" );
    auto n2 = graph->add_node( "b" );

    EXPECT_EQ( 0, graph->neighbours( n1 ).size());
    EXPECT_EQ( 0, graph->neighbours( n2 ).size() );
}

TEST_F(TestGraph, ToNodeOfEdgeIsNeighbourOfFromNode ) {
    auto from = graph->add_node( "a" );
    auto to = graph->add_node( "b" );
    graph->add_edge( from, to, 1.0 );

    auto nbr = graph->neighbours( from );
    EXPECT_EQ( 1, nbr.size() );
    EXPECT_EQ( to, nbr[0] );
}

TEST_F(TestGraph, FromNodeOfEdgeIsNotNeighbourOfToNode ) {
    auto from = graph->add_node( "a" );
    auto to = graph->add_node( "b" );
    graph->add_edge( from, to, 1.0 );

    auto nbr = graph->neighbours( to );
    EXPECT_EQ( 0, nbr.size() );
}

TEST_F(TestGraph, FromNodeOfEdgeIsNeighbourOfToNodeInUndirectedGraph ) {
    auto from = undirected_graph->add_node( "a" );
    auto to = undirected_graph->add_node( "b" );
    undirected_graph->add_edge( from, to, 1.0 );

    auto nbr = undirected_graph->neighbours( to );
    EXPECT_EQ( nbr.size(), 1 );
    EXPECT_EQ( from, nbr[0] );
}

TEST_F(TestGraph, RemoveNodeAlsoRemovesEdges ) {
    auto from = graph->add_node( "a" );
    auto to = graph->add_node( "b" );
    graph->add_edge( from, to, 1.0 );
    EXPECT_EQ(1, graph->num_edges());
    EXPECT_TRUE(graph->has_edge(from, to));

    graph->remove_node(from);
    EXPECT_EQ(0, graph->num_edges());
}

TEST_F(TestGraph, RemoveNodeOnlyRemovesIncidentEdges ) {
    auto a = undirected_graph->add_node( "a" );
    auto b = undirected_graph->add_node( "b" );
    auto c = undirected_graph->add_node( "c" );
    undirected_graph->add_edge( a, b, 1.0 );
    undirected_graph->add_edge( a, c, 1.0 );
    undirected_graph->add_edge( b, c, 1.0 );

    EXPECT_EQ(6, undirected_graph->num_edges());
    EXPECT_TRUE(undirected_graph->has_edge(a, b));
    EXPECT_TRUE(undirected_graph->has_edge(b, a));
    EXPECT_TRUE(undirected_graph->has_edge(a, c));
    EXPECT_TRUE(undirected_graph->has_edge(c, a));
    EXPECT_TRUE(undirected_graph->has_edge(b, c));
    EXPECT_TRUE(undirected_graph->has_edge(c, b));

    undirected_graph->remove_node(b);
    EXPECT_EQ(2, undirected_graph->num_edges());
    EXPECT_TRUE(undirected_graph->has_edge(a, c));
    EXPECT_TRUE(undirected_graph->has_edge(c, a));
}

void dump_graph(const TestGraph::GraphPtr & graph) {
    for( const auto & n : graph->nodes()) {
        std::cout << " Node : " << n->data() << std::endl;
    }
    for( const auto & e : graph->edges()) {
        std::cout << " Edge ( " << e.from()->data() << " -> " << e.to()->data() << " )" << std::endl;
    }
}

TEST_F(TestGraph, RemoveNodeRemovesItAsTargetForEdges ) {
    auto node_a = undirected_graph->add_node( "a" );
    auto node_b = undirected_graph->add_node( "b" );
    auto node_c = undirected_graph->add_node( "c" );
    undirected_graph->add_edge( node_a, node_b, 1.0 );
    undirected_graph->add_edge( node_b, node_c, 1.0 );

    undirected_graph->remove_node(node_a);
    EXPECT_EQ(undirected_graph->num_nodes(), 2);
    EXPECT_EQ(undirected_graph->num_edges(), 2); //b->c, c->b

    undirected_graph->remove_node(node_c);
    EXPECT_EQ(undirected_graph->num_nodes(), 1);
    EXPECT_EQ(undirected_graph->num_edges(), 0);

    undirected_graph->remove_node(node_b);
    EXPECT_EQ(undirected_graph->num_nodes(), 0);
    EXPECT_EQ(undirected_graph->num_edges(), 0);
}

TEST_F(TestGraph, GraphAssignmentWorks ) {
    auto g2 = undirected_graph;
}
