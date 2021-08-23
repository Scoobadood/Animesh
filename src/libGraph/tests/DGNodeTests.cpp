//
// Created by Dave Durbin (Old) on 18/8/21.
//

#include "DGNodeTests.h"

#include "TestUtilities.h"

#include <memory>

void DirectedGraphNodeTests::SetUp() {
  using namespace animesh;

  graph = std::make_shared<Graph<std::string, float>>(true);

  gn1 = std::make_shared<GraphNode>("a");
  gn2 = std::make_shared<GraphNode>("b");
}

void DirectedGraphNodeTests::TearDown() {}

/* **********************************************************************
 * *                                                                    *
 * * Graph Constructor tests                                            *
 * *                                                                    *
 * **********************************************************************/

TEST_F(DirectedGraphNodeTests, add_node_by_data_to_empty_graph_should_increase_node_count) {
  graph->add_node("a");

  EXPECT_EQ(graph->num_nodes(), 1);
}

TEST_F(DirectedGraphNodeTests, add_duplicate_node_by_data_should_increase_node_count_twice) {
  graph->add_node("a");
  graph->add_node("a");
  EXPECT_EQ(graph->num_nodes(), 2);
}

TEST_F(DirectedGraphNodeTests, add_different_nodes_by_data_should_increase_node_count_twice) {
  graph->add_node("a");
  graph->add_node("b");
  EXPECT_EQ(graph->num_nodes(), 2);
}

TEST_F(DirectedGraphNodeTests, add_node_by_value_to_empty_graph_should_increase_node_count) {
  graph->add_node(gn1);
  EXPECT_EQ(graph->num_nodes(), 1);
}

TEST_F(DirectedGraphNodeTests, add_duplicate_node_by_value_should_throw) {
  graph->add_node(gn1);
  EXPECT_THROW_WITH_MESSAGE(
      graph->add_node(gn1),
      std::runtime_error,
      "Node 0x[a-z0-9]+ \\(a\\) already exists"
      );
}

TEST_F(DirectedGraphNodeTests, add_node_by_value_to_existing_graph_should_increase_node_count) {
  graph->add_node(gn1);
  EXPECT_EQ(graph->num_nodes(), 1);
  graph->add_node(gn2);
  EXPECT_EQ(graph->num_nodes(), 2);
}

TEST_F(DirectedGraphNodeTests, remove_node_also_removes_edges_from_node) {
  graph->add_node(gn1);
  graph->add_node(gn2);
  graph->add_edge(gn1, gn2, 1.0);
  graph->remove_node(gn1);
  EXPECT_EQ(graph->num_edges(), 0);
}

TEST_F(DirectedGraphNodeTests, remove_node_also_removes_edges_to_node) {
  graph->add_node(gn1);
  graph->add_node(gn2);
  graph->add_edge(gn1, gn2, 1.0);
  graph->remove_node(gn2);
  EXPECT_EQ(graph->num_edges(), 0);
}

TEST_F(DirectedGraphNodeTests, remove_node_also_removes_all_edges_incident_at_node) {
  graph->add_node(gn1);
  graph->add_node(gn2);
  auto c = graph->add_node("c");
  graph->add_edge(gn1, gn2, 1.0);
  graph->add_edge(gn2, c, 1.0);
  graph->add_edge(c, gn1, 1.0);
  graph->add_edge(gn1, c, 1.0);
  graph->add_edge(c, gn2, 1.0);
  graph->add_edge(gn2, gn1, 1.0);

  graph->remove_node(gn1);
  EXPECT_EQ(graph->num_edges(), 2);
  EXPECT_TRUE(graph->has_edge(gn2, c));
  EXPECT_TRUE(graph->has_edge(c, gn2));
  EXPECT_FALSE(graph->has_edge(gn1, c));
  EXPECT_FALSE(graph->has_edge(gn1, gn2));
  EXPECT_FALSE(graph->has_edge(gn2, gn1));
  EXPECT_FALSE(graph->has_edge(c, gn1));
}

TEST_F(DirectedGraphNodeTests, remove_missing_node_should_throw) {
  EXPECT_THROW_WITH_MESSAGE(
      graph->remove_node(gn2),
      std::runtime_error,
      "No node 0x[a-z0-9]+ \\(b\\)"
      );
}

TEST_F(DirectedGraphNodeTests, node_with_no_edges_has_no_neighbours ) {
  graph->add_node(gn1);
  graph->add_node(gn2);

  EXPECT_EQ( graph->neighbours( gn1 ).size(), 0);
  EXPECT_EQ( graph->neighbours( gn2 ).size(), 0);
}
