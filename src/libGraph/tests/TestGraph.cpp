#include <Graph/Graph.h>

#include "TestGraph.h"
#include "TestUtilities.h"
#include <memory>

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

void dump_graph(const TestGraph::GraphPtr & graph) {
    for( const auto & n : graph->nodes()) {
        std::cout << " Node : " << n->data() << std::endl;
    }
    for( const auto & e : graph->edges()) {
        std::cout << " Edge ( " << e.from()->data() << " -> " << e.to()->data() << " )" << std::endl;
    }
}

TEST_F(TestGraph, GraphAssignmentWorks ) {
    auto g2 = undirected_graph;
}

TEST_F(TestGraph, copy_constructor_should_copy_graph ) {
  graph->add_node(gn1);
  graph->add_node(gn2);
  graph->add_edge(gn1, gn2, 1.1);
  animesh::Graph<std::string, float> graph2{*graph};

  EXPECT_EQ(graph2.num_edges(), graph->num_edges());
  EXPECT_EQ(graph2.num_nodes(), graph->num_nodes());
}

