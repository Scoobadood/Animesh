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
