//
// Created by Dave Durbin (Old) on 18/8/21.
//

#include "UDGNodeTests.h"

#include "TestUtilities.h"

#include <memory>

void UndirectedGraphNodeTests::SetUp() {
  using namespace animesh;

  graph = std::make_shared<Graph<std::string, float>>(false);

  gn1 = std::make_shared<GraphNode>("a");
  gn2 = std::make_shared<GraphNode>("b");
}

void UndirectedGraphNodeTests::TearDown() {}

/* **********************************************************************
 * *                                                                    *
 * * Graph Constructor tests                                            *
 * *                                                                    *
 * **********************************************************************/

TEST_F(UndirectedGraphNodeTests, add_node_by_data_to_empty_graph_should_increase_node_count) {
  graph->add_node("a");

  EXPECT_EQ(graph->num_nodes(), 1);
}

TEST_F(UndirectedGraphNodeTests, add_duplicate_node_by_data_should_increase_node_count_twice) {
  graph->add_node("a");
  graph->add_node("a");
  EXPECT_EQ(graph->num_nodes(), 2);
}

TEST_F(UndirectedGraphNodeTests, add_different_nodes_by_data_should_increase_node_count_twice) {
  graph->add_node("a");
  graph->add_node("b");
  EXPECT_EQ(graph->num_nodes(), 2);
}

TEST_F(UndirectedGraphNodeTests, add_node_by_value_to_empty_graph_should_increase_node_count) {
  graph->add_node(gn1);
  EXPECT_EQ(graph->num_nodes(), 1);
}

TEST_F(UndirectedGraphNodeTests, add_duplicate_node_by_value_should_not_increase_node_count) {
  graph->add_node(gn1);
  EXPECT_EQ(graph->num_nodes(), 1);
  graph->add_node(gn1);
  EXPECT_EQ(graph->num_nodes(), 1);
}

TEST_F(UndirectedGraphNodeTests, add_node_by_value_to_existing_graph_should_increase_node_count) {
  graph->add_node(gn1);
  EXPECT_EQ(graph->num_nodes(), 1);
  graph->add_node(gn2);
  EXPECT_EQ(graph->num_nodes(), 2);
}

TEST_F(UndirectedGraphNodeTests, remove_node_also_removes_edges_from_node) {
  auto from = graph->add_node("a");
  auto to = graph->add_node("b");
  graph->add_edge(from, to, 1.0);

  graph->remove_node(from);
  EXPECT_EQ(graph->num_edges(), 0);
}

TEST_F(UndirectedGraphNodeTests, remove_node_also_removes_edges_to_node) {
  graph->add_node(gn1);
  graph->add_node(gn2);
  graph->add_edge(gn1, gn2, 1.0);

  graph->remove_node(gn2);
  EXPECT_EQ(graph->num_edges(), 0);
}

TEST_F(UndirectedGraphNodeTests, remove_node_also_removes_all_edges_incident_at_node) {
  auto a = graph->add_node("a");
  auto b = graph->add_node("b");
  auto c = graph->add_node("c");
  graph->add_edge(a, b, 1.0);
  graph->add_edge(b, c, 1.0);
  graph->add_edge(c, a, 1.0);

  graph->remove_node(a);
  EXPECT_EQ(graph->num_edges(), 1);
  EXPECT_TRUE(graph->has_edge(b, c));
  EXPECT_TRUE(graph->has_edge(c, b));
  EXPECT_FALSE(graph->has_edge(a, c));
  EXPECT_FALSE(graph->has_edge(a, b));
  EXPECT_FALSE(graph->has_edge(b, a));
  EXPECT_FALSE(graph->has_edge(c, a));
}

TEST_F(UndirectedGraphNodeTests, remove_missing_node_should_fail) {
  auto node_a = graph->add_node("a");
  auto node_b = graph->add_node("b");
  auto before_remove = graph->num_nodes();
  graph->remove_node(gn1);
  auto after_remove = graph->num_nodes();
  EXPECT_EQ(before_remove, after_remove);

}