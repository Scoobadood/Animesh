//
// Created by Dave Durbin (Old) on 20/8/21.
//

#include "UDGEdgeTests.h"
#include "TestUtilities.h"

void UndirectedGraphEdgeTests::SetUp() {
  using namespace animesh;

  graph = std::make_shared<Graph<std::string, float>>(false);
  gn1 = std::make_shared<GraphNode>("a");
  gn2 = std::make_shared<GraphNode>("b");
  graph->add_node(gn1);
  graph->add_node(gn2);
}

void UndirectedGraphEdgeTests::TearDown() {}

TEST_F(UndirectedGraphEdgeTests, add_edge_to_empty_graph_should_increase_edge_count) {
  graph->add_edge(gn1, gn2, 1.0f);
  EXPECT_EQ(graph->num_edges(), 1);
}

TEST_F(UndirectedGraphEdgeTests, add_edge_to_non_empty_graph_should_increase_edge_count) {
  graph->add_edge(gn1, gn2, 1.0f);
  auto gn3 = graph->add_node("c");
  graph->add_edge(gn1, gn3, 1.0f);
  EXPECT_EQ(graph->num_edges(), 2);
}

TEST_F(UndirectedGraphEdgeTests, add_duplicate_edge_should_not_increase_edge_count) {
  graph->add_edge(gn1, gn2, 1.0);
  size_t before_count = graph->num_edges();

  graph->add_edge(gn1, gn2, 1.0);
  size_t after_count = graph->num_edges();

  EXPECT_EQ(before_count, after_count);
}

TEST_F(UndirectedGraphEdgeTests, add_reverse_edge_should_not_increase_edge_count) {
  graph->add_edge(gn1, gn2, 1.0);
  size_t before_count = graph->num_edges();

  graph->add_edge(gn2, gn1, 1.0);
  size_t after_count = graph->num_edges();

  EXPECT_EQ(after_count, before_count);
}

TEST_F(UndirectedGraphEdgeTests, delete_reverse_of_added_edge_reduces_edge_count) {
  graph->add_edge(gn1, gn2, 1.0);
  size_t before_count = graph->num_edges();

  graph->remove_edge(gn2, gn1);
  size_t after_count = graph->num_edges();

  EXPECT_EQ(after_count, before_count-1);
}

TEST_F(UndirectedGraphEdgeTests, to_node_of_edge_is_neighbour_of_from_node) {
  graph->add_edge(gn1, gn2, 1.0);

  auto nbr = graph->neighbours(gn1);
  EXPECT_EQ(1, nbr.size());
  EXPECT_EQ(gn2, nbr[0]);
}

TEST_F(UndirectedGraphEdgeTests, from_node_is_neighbour_of_to_node) {
  graph->add_edge(gn1, gn2, 1.0);

  auto nbr = graph->neighbours(gn2);
  EXPECT_EQ(gn1, nbr[0]);
}

TEST_F(UndirectedGraphEdgeTests, forward_edge_is_retrievable_in_forward_order) {
  graph->add_edge(gn1, gn2, 1.0);

  auto edge = graph->edge(gn1, gn2);
  EXPECT_NE(edge, nullptr);
  EXPECT_EQ(*edge, 1.0);
}

TEST_F(UndirectedGraphEdgeTests, forward_edge_is_retrievable_in_backward_order) {
  graph->add_edge(gn1, gn2, 1.0);

  auto edge = graph->edge(gn2, gn1);
  EXPECT_NE(edge, nullptr);
  EXPECT_EQ(*edge, 1.0);
}
