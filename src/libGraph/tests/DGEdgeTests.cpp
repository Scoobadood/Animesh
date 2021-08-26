//
// Created by Dave Durbin (Old) on 19/8/21.
//

#include "DGEdgeTests.h"
#include "TestUtilities.h"
#include "gmock/gmock.h"
#include "gmock/gmock-matchers.h"

void DirectedGraphEdgeTests::SetUp() {
  using namespace animesh;

  graph = std::make_shared<Graph<std::string, float>>(true);
  gn1 = graph->add_node("a");
  gn2 = graph->add_node("b");
  gn3 = graph->add_node("c");
  gn4 = graph->add_node("d");
}

void DirectedGraphEdgeTests::TearDown() {}

void DirectedGraphEdgeTests::assertContainsInAnyOrder(
    const std::vector<const GraphNodePtr> &nodes,
    const std::vector<std::string> &expected) {
  using namespace std;
  if (nodes.size() != expected.size()) {
    FAIL() << "Nodes and expected are different sizes";
  }
  vector<string> node_data;
  for (const auto &n : nodes) {
    node_data.push_back(n->data());
  }
  sort(begin(node_data), end(node_data));
  for (const auto &s : expected) {
    if (!binary_search(begin(node_data), end(node_data), s)) {
      FAIL() << "Didn't find " << s << " in results";
    }
  }
}

TEST_F(DirectedGraphEdgeTests, add_edge_to_empty_graph_should_increase_edge_count) {
  graph->add_edge(gn1, gn2, 1.0f);
  EXPECT_EQ(graph->num_edges(), 1);
}

TEST_F(DirectedGraphEdgeTests, add_edge_to_non_empty_graph_should_increase_edge_count) {
  graph->add_edge(gn1, gn2, 1.0f);
  auto num_edges = graph->num_edges();
  graph->add_edge(gn1, gn3, 1.0f);
  EXPECT_EQ(num_edges + 1, graph->num_edges());
}

TEST_F(DirectedGraphEdgeTests, add_edge_creates_forward_edge) {
  graph->add_edge(gn1, gn2, 1.0f);
  EXPECT_TRUE(graph->has_edge(gn1, gn2));
}

TEST_F(DirectedGraphEdgeTests, add_edge_does_not_create_reverse_edge) {
  graph->add_edge(gn1, gn2, 1.0f);
  EXPECT_FALSE(graph->has_edge(gn2, gn1));
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
  auto num_edges = graph->num_edges();
  graph->add_edge(gn2, gn1, 2.0);
  EXPECT_EQ(num_edges + 1, graph->num_edges());
}

TEST_F(DirectedGraphEdgeTests, remove_edge_reduces_edge_count) {
  graph->add_edge(gn1, gn2, 1.0);
  size_t num_edges = graph->num_edges();

  graph->remove_edge(gn1, gn2);
  EXPECT_EQ(num_edges - 1, graph->num_edges());
}

TEST_F(DirectedGraphEdgeTests, remove_edge_removes_edge) {
  graph->add_edge(gn1, gn2, 1.0);

  graph->remove_edge(gn1, gn2);
  EXPECT_FALSE(graph->has_edge(gn1, gn2));
}

TEST_F(DirectedGraphEdgeTests, remove_edge_does_not_remove_reverse_edge) {
  graph->add_edge(gn1, gn2, 1.0);
  graph->add_edge(gn2, gn1, 1.1);

  graph->remove_edge(gn1, gn2);
  EXPECT_TRUE(graph->has_edge(gn2, gn1));
}

TEST_F(DirectedGraphEdgeTests, remove_reverse_edge_should_throw) {
  graph->add_edge(gn1, gn2, 1.0);
  EXPECT_THROW_WITH_MESSAGE(
      graph->remove_edge(gn2, gn1),
      std::runtime_error,
      "No edge from 0x[a-z0-9]+ \\(b\\) to 0x[a-z0-9]+ \\(a\\)");
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

TEST_F(DirectedGraphEdgeTests, collapse_edge_throws_when_edge_is_missing) {
  EXPECT_THROW_WITH_MESSAGE(
      graph->collapse_edge(gn1, gn2, node_merge_function, edge_merge_function),
      std::runtime_error,
      "No edge from 0x[a-z0-9]+ \\(a\\) to 0x[a-z0-9]+ \\(b\\)");
}

TEST_F(DirectedGraphEdgeTests, collapse_edge_invokes_node_merge_function) {
  graph->add_edge(gn1, gn2, 1.0f);
  auto n = graph->add_node("c");
  graph->add_edge(n, gn1, 1.0f);

  testing::MockFunction<std::string(const std::string &a, const std::string &b)> mock_node_merge_function;

  EXPECT_CALL(mock_node_merge_function, Call("a", "b")).Times(1);
  graph->collapse_edge(gn1, gn2, mock_node_merge_function.AsStdFunction(), edge_merge_function);
}

TEST_F(DirectedGraphEdgeTests, collapse_isolated_edge_merges_nodes) {
  graph->add_edge(gn1, gn2, 1.0f);
  auto num_nodes = graph->num_nodes();
  graph->collapse_edge(gn1, gn2, node_merge_function, edge_merge_function);

  EXPECT_EQ(num_nodes - 1, graph->num_nodes());
  assertContainsInAnyOrder(graph->nodes(), std::vector<std::string>{"(a+b)", "c", "d"});
}

TEST_F(DirectedGraphEdgeTests, collapse_any_edge_merges_nodes) {
  graph->add_edge(gn1, gn2, 1.0f);
  graph->add_edge(gn3, gn1, 1.0f);
  graph->add_edge(gn4, gn2, 1.0f);
  auto num_nodes = graph->num_nodes();

  graph->collapse_edge(gn1, gn2, node_merge_function, edge_merge_function);

  EXPECT_EQ(num_nodes - 1, graph->num_nodes());
  assertContainsInAnyOrder(graph->nodes(), std::vector<std::string>{"c", "d", "(a+b)"});
}

TEST_F(DirectedGraphEdgeTests, collapse_edge_removes_edge) {
  graph->add_edge(gn1, gn2, 1.0f);
  graph->collapse_edge(gn1, gn2, node_merge_function, edge_merge_function);

  EXPECT_EQ(0, graph->num_edges());
}

TEST_F(DirectedGraphEdgeTests, collapse_edge_creates_edges_from_neighbouring_nodes) {
  graph->add_edge(gn1, gn2, 0.3f);
  graph->add_edge(gn3, gn1, 0.5f);
  graph->add_edge(gn4, gn2, 0.7f);

  graph->collapse_edge(gn1, gn2, node_merge_function, edge_merge_function);

  EXPECT_EQ(2, graph->num_edges());
  // Edge 1 is between gn3 and (a+b)
  auto an_edge = graph->edges()[0];
  GraphNodePtr new_node;
  if (an_edge.from() == gn3 || an_edge.from() == gn4) {
    new_node = an_edge.to();
  } else {
    new_node = an_edge.from();
  }

  EXPECT_EQ(new_node->data(), "(a+b)");
  EXPECT_TRUE(graph->has_edge(gn3, new_node));
  EXPECT_TRUE(graph->has_edge(gn4, new_node));
}

TEST_F(DirectedGraphEdgeTests, node_with_edges_to_node_returns_inbound_edges) {
  graph->add_edge(gn1, gn2, 1.2f);
  graph->add_edge(gn3, gn2, 3.2f);

  auto inbound_edges_and_nodes = graph->nodes_with_edges_to(gn2);
  EXPECT_EQ(2, inbound_edges_and_nodes.size());
  EXPECT_EQ(1.2f, *(inbound_edges_and_nodes[gn1]));
  EXPECT_EQ(3.2f, *(inbound_edges_and_nodes[gn3]));
}

TEST_F(DirectedGraphEdgeTests, node_with_edges_from_node_returns_outbound_edges) {
  graph->add_edge(gn2, gn1, 2.1f);
  graph->add_edge(gn2, gn3, 2.3f);

  auto outbound_edges_and_nodes = graph->nodes_with_edges_from(gn2);
  EXPECT_EQ(2, outbound_edges_and_nodes.size());
  EXPECT_EQ(2.1f, *(outbound_edges_and_nodes[gn1]));
  EXPECT_EQ(2.3f, *(outbound_edges_and_nodes[gn3]));
}

TEST_F(DirectedGraphEdgeTests, node_with_edges_from_node_does_not_return_inbound_edges) {
  graph->add_edge(gn1, gn2, 1.2f);
  graph->add_edge(gn2, gn3, 3.2f);

  auto outbound_edges_and_nodes = graph->nodes_with_edges_from(gn2);
  EXPECT_EQ(1, outbound_edges_and_nodes.size());
  EXPECT_EQ(3.2f, *(outbound_edges_and_nodes[gn3]));
}

TEST_F(DirectedGraphEdgeTests, node_with_edges_to_node_does_not_return_outbound_edges) {
  graph->add_edge(gn1, gn2, 2.1f);
  graph->add_edge(gn2, gn3, 2.3f);

  auto inbound_edges_and_nodes = graph->nodes_with_edges_to(gn2);
  EXPECT_EQ(1, inbound_edges_and_nodes.size());
  EXPECT_EQ(2.1f, *(inbound_edges_and_nodes[gn1]));
}

TEST_F(DirectedGraphEdgeTests, collapse_singular_edge_does_not_call_edge_merge_function) {
  using namespace testing;
  using namespace std;

  graph->add_edge(gn1, gn2, 1.1);
  graph->add_edge(gn2, gn3, 1.1);
  graph->add_edge(gn3, gn4, 1.1);
  graph->add_edge(gn4, gn1, 1.1);

  MockFunction<float(const float &f1, const float &f2)> mock_edge_merge_function;
  EXPECT_CALL(
      mock_edge_merge_function,
      Call(_, _)
  ).Times(0);
  graph->collapse_edge(gn1, gn2, node_merge_function, mock_edge_merge_function.AsStdFunction());
}

TEST_F(DirectedGraphEdgeTests, collapse_edge_invokes_edge_merge_function_on_incident_edges) {
  graph->add_edge(gn3, gn1, 0.5f);
  graph->add_edge(gn3, gn2, 0.7f);
  graph->add_edge(gn1, gn2, 1.1f);
  graph->add_edge(gn1, gn4, 1.3f);
  graph->add_edge(gn2, gn4, 1.7f);

  testing::MockFunction<float(const float &f1, const float &f2)> mock_edge_merge_function;

  EXPECT_CALL(mock_edge_merge_function, Call(1.3, 1.7)).Times(1);
  EXPECT_CALL(mock_edge_merge_function, Call(0.5, 0.7)).Times(1);
  graph->collapse_edge(gn1, gn2, node_merge_function, mock_edge_merge_function.AsStdFunction());
}

TEST_F(DirectedGraphEdgeTests, collapse_edge_invokes_edge_merge_function_on_inbound_edges) {
  graph->add_edge(gn3, gn1, 0.5f);
  graph->add_edge(gn3, gn2, 0.7f);
  graph->add_edge(gn1, gn2, 1.1f);

  graph->collapse_edge(gn1, gn2, node_merge_function, edge_merge_function);
  EXPECT_EQ(1, graph->num_edges());
  auto edge = graph->edges()[0];
  EXPECT_EQ(gn3, edge.from());
  EXPECT_EQ(1.2f, *(edge.data()));
}

TEST_F(DirectedGraphEdgeTests, collapse_edge_invokes_edge_merge_function_on_outbound_edges) {
  graph->add_edge(gn1, gn2, 1.1f);
  graph->add_edge(gn1, gn4, 1.3f);
  graph->add_edge(gn2, gn4, 1.7f);

  graph->collapse_edge(gn1, gn2, node_merge_function, edge_merge_function);
  EXPECT_EQ(1, graph->num_edges());
  auto edge = graph->edges()[0];
  EXPECT_EQ(gn4, edge.to());
  EXPECT_EQ(3.0f, *(edge.data()));
}