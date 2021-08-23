//
// Created by Dave Durbin (Old) on 19/8/21.
//

#include "DGEdgeTests.h"
#include "TestUtilities.h"

void DirectedGraphEdgeTests::SetUp() {
  using namespace animesh;

  graph = std::make_shared<Graph<std::string, float>>(true);
  gn1 = std::make_shared<GraphNode>("a");
  gn2 = std::make_shared<GraphNode>("b");
  graph->add_node(gn1);
  graph->add_node(gn2);
}

void DirectedGraphEdgeTests::TearDown() {}

TEST_F(DirectedGraphEdgeTests, add_edge_to_empty_graph_should_increase_edge_count) {
  graph->add_edge(gn1, gn2, 1.0f);
  EXPECT_EQ(graph->num_edges(), 1);
}

TEST_F(DirectedGraphEdgeTests, add_edge_to_non_empty_graph_should_increase_edge_count) {
  graph->add_edge(gn1, gn2, 1.0f);
  auto gn3 = graph->add_node("c");
  graph->add_edge(gn1, gn3, 1.0f);
  EXPECT_EQ(graph->num_edges(), 2);
}

TEST_F(DirectedGraphEdgeTests, add_duplicate_edge_should_throw) {
  graph->add_edge(gn1, gn2, 1.0);

  EXPECT_THROW_WITH_MESSAGE(
      graph->add_edge(gn1, gn2, 1.0),
      std::runtime_error,
      R"(Edge already exists from 0x[0-9a-z]+ \(a\) to 0x[a-z0-9]+ \(b\))"
  );
}

TEST_F(DirectedGraphEdgeTests, add_reverse_edge_should_increase_edge_count) {
  graph->add_edge(gn1, gn2, 1.0);
  size_t before_count = graph->num_edges();

  graph->add_edge(gn2, gn1, 1.0);
  size_t after_count = graph->num_edges();

  EXPECT_EQ(after_count, before_count + 1);
}

TEST_F(DirectedGraphEdgeTests, to_node_of_edge_is_neighbour_of_from_node) {
  graph->add_edge(gn1, gn2, 1.0);

  auto nbr = graph->neighbours(gn1);
  EXPECT_EQ(1, nbr.size());
  EXPECT_EQ(gn2, nbr[0]);
}

TEST_F(DirectedGraphEdgeTests, from_node_is_not_neighbour_of_to_node) {
  graph->add_edge(gn1, gn2, 1.0);

  auto nbr = graph->neighbours(gn2);
  EXPECT_TRUE(nbr.empty());
}

TEST_F(DirectedGraphEdgeTests, forward_edge_is_retrievable_in_forward_order) {
  graph->add_edge(gn1, gn2, 1.0);

  auto edge = graph->edge(gn1, gn2);
  EXPECT_NE(edge, nullptr);
  EXPECT_EQ(*edge, 1.0);
}

TEST_F(DirectedGraphEdgeTests, forward_edge_is_not_retrievable_in_backward_order) {
  graph->add_edge(gn1, gn2, 1.0);

  EXPECT_THROW_WITH_MESSAGE(
      graph->edge(gn2, gn1),
      std::runtime_error,
      "No edge from 0x[a-z0-9]+ \\(b\\) to 0x[a-z0-9]+ \\(a\\)");
}
