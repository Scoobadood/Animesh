//
// Created by Dave Durbin (Old) on 30/8/21.
//

#include "TestCycleExtractor.h"

#include <Graph/CycleExtractor.h>

//  v0        v1
//  +---->----+
//  |         |
//  V         ^
//  |         |
//  +---->----+
//  v2        v3
void setup_graph_one_loop(std::shared_ptr<typename animesh::Graph<std::string, int>> graph) {
  auto node0 = graph->add_node("v_0");
  auto node1 = graph->add_node("v_1");
  auto node2 = graph->add_node("v_2");
  auto node3 = graph->add_node("v_3");
  graph->add_edge(node0, node1, 1);
  graph->add_edge(node3, node1, 2);
  graph->add_edge(node2, node3, 3);
  graph->add_edge(node0, node2, 4);
}

/*
 * v0        v1        v2
 * +---->----+----->---+
 * |         |         |
 * ^         ^         v
 * |         |         |
 * +---->----+----<----+
 * v5        v4        v3
 */
void setup_graph_double_loop(std::shared_ptr<typename animesh::Graph<std::string, int>> graph) {
  auto node0 = graph->add_node("v_0");
  auto node1 = graph->add_node("v_1");
  auto node2 = graph->add_node("v_2");
  auto node3 = graph->add_node("v_3");
  auto node4 = graph->add_node("v_4");
  auto node5 = graph->add_node("v_5");
  graph->add_edge(node0, node1, 1);
  graph->add_edge(node1, node2, 2);
  graph->add_edge(node5, node0, 3);
  graph->add_edge(node4, node1, 4);
  graph->add_edge(node2, node3, 5);
  graph->add_edge(node5, node4, 6);
  graph->add_edge(node3, node4, 7);
}

/*
 * v0    v1        v2
 * +-->--+----->---+
 * |    /|         |
 * ^  /  ^         v
 * |/    |         |
 * +-->--+----<----+
 * v5    v4        v3
 */
void setup_graph_double_loop_diagonal(std::shared_ptr<typename animesh::Graph<std::string, int>> graph) {
  auto node0 = graph->add_node("v_0");
  auto node1 = graph->add_node("v_1");
  auto node2 = graph->add_node("v_2");
  auto node3 = graph->add_node("v_3");
  auto node4 = graph->add_node("v_4");
  auto node5 = graph->add_node("v_5");
  graph->add_edge(node0, node1, 1);
  graph->add_edge(node1, node2, 2);
  graph->add_edge(node5, node0, 3);
  graph->add_edge(node4, node1, 4);
  graph->add_edge(node2, node3, 5);
  graph->add_edge(node5, node4, 6);
  graph->add_edge(node3, node4, 7);
  graph->add_edge(node5, node1, 8);
}

void TestCycleExtractor::SetUp() {
  using namespace animesh;

  graph = std::make_shared<Graph<std::string, int>>(true);
  undirected_graph = std::make_shared<Graph<std::string, int>>();
}

void TestCycleExtractor::TearDown() {}

TEST_F(TestCycleExtractor, extract_cycles_from_directed_graph) {
  setup_graph_one_loop(graph);
  std::set<std::vector<GraphNodePtr>> cycles;
  animesh::CycleExtractor<std::string, int>::extract_cycles(graph, cycles);

  // Will find cycles in both directions
  EXPECT_EQ(1, cycles.size());
}

TEST_F(TestCycleExtractor, extract_cycles_from_undirected_graph) {
  setup_graph_one_loop(undirected_graph);
  std::set<std::vector<GraphNodePtr>> cycles;
  animesh::CycleExtractor<std::string, int>::extract_cycles(undirected_graph, cycles);

  // Will find cycles in both directions
  EXPECT_EQ(1, cycles.size());
}

TEST_F(TestCycleExtractor, extract_cycles_from_directed_graph_double_loop) {
  setup_graph_double_loop(graph);
  std::set<std::vector<GraphNodePtr>> cycles;
  animesh::CycleExtractor<std::string, int>::extract_cycles(graph, cycles);

  for (const auto &c : cycles) {
    for (const auto &n : c) {
      std::cout << n->data() << " ";
    }
    std::cout << std::endl;
  }

  // Will find cycles in both directions
  EXPECT_EQ(2, cycles.size());
}

TEST_F(TestCycleExtractor, extract_cycles_from_undirected_graph_double_loop) {
  setup_graph_double_loop(undirected_graph);
  std::set<std::vector<GraphNodePtr>> cycles;
  animesh::CycleExtractor<std::string, int>::extract_cycles(undirected_graph, cycles);

  // Will find cycles in both directions
  EXPECT_EQ(2, cycles.size());
}

TEST_F(TestCycleExtractor, extract_cycles_from_directed_graph_with_diagonals) {
  setup_graph_double_loop_diagonal(graph);
  std::set<std::vector<GraphNodePtr>> cycles;
  animesh::CycleExtractor<std::string, int>::extract_cycles(graph, cycles);

  std::cout << " number of cycles found " << cycles.size() << std::endl;
//  int n = 0;
  for (const auto & cycle : cycles) {
//    std::cout << "Cycle " << n << ":";
    for (const auto &node : cycle) {
      std::cout << node->data() << " ";
    }
    std::cout << std::endl;
//    n++;
  }
  // Will find cycles in both directions
  EXPECT_EQ(3, cycles.size());
}

TEST_F(TestCycleExtractor, extract_cycles_from_undirected_graph_with_diagonals) {
  setup_graph_double_loop_diagonal(undirected_graph);
  std::set<std::vector<GraphNodePtr>> cycles;
  animesh::CycleExtractor<std::string, int>::extract_cycles(undirected_graph, cycles);

  for (const auto &c : cycles) {
    std::cout << std::endl;
    for (const auto &n : c) {
      std::cout << n->data() << " ";
    }
  }
  // Will find cycles in both directions
  EXPECT_EQ(3, cycles.size());
}
