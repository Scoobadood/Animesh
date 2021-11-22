#include "TestSurfel.h"
#include <Surfel/Surfel.h>
#include <Surfel/FrameData.h>
#include <Surfel/Surfel_IO.h>
#include <Surfel/SurfelGraph.h>
#include <Graph/Graph.h>
#include <Eigen/Core>
#include <gtest/gtest.h>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <fstream>

void TestSurfel::SetUp() {
  std::default_random_engine rng{123};
  m_surfel_builder = new SurfelBuilder(rng);
}

void TestSurfel::TearDown() {
  delete m_surfel_builder;
}

void TestSurfelIO::SetUp() {
  using namespace std;

  std::default_random_engine rng{123};
  m_surfel_builder = new SurfelBuilder(rng);
  surfel_graph = make_shared<SurfelGraph>();
  Eigen::Matrix3f transform;
  transform << 1.1f, 2.2f, 3.3f,
      4.4f, 5.5f, 6.6f,
      7.7f, 8.8f, 9.9f;

  m_surfel_builder
      ->reset()
      ->with_id("a")
      ->with_frame({
                       {1, 1, 1}, // pif
                       1.1f, // depth
                       transform,
                       {1.1f, 2.2f, 3.3f}, // norm
                       {1.1f, 2.2f, 3.3f} //pos
                   }
      )
      ->with_tangent(1.0f, 0.0f, 0.0f)
      ->with_reference_lattice_offset(0.15f, 0.25f);
  const auto s1 = make_shared<Surfel>(m_surfel_builder->build());
  s1->set_rosy_smoothness(0);
  s1->set_posy_smoothness(22.9);

  m_surfel_builder
      ->reset()
      ->with_id("b")
      ->with_frame({
                       {1, 1, 1}, // pif
                       1.1f, // depth
                       transform,
                       {1.1f, 2.2f, 3.3f}, // norm
                       {1.1f, 2.2f, 3.3f} //pos
                   }
      )
      ->with_tangent(
          M_SQRT1_2, 0, M_SQRT1_2)
      ->with_reference_lattice_offset(0.35f, 0.45f);
  const auto s2 = make_shared<Surfel>(m_surfel_builder->build());

  s2->set_rosy_smoothness(0);
  s2->set_posy_smoothness(99.2);
  const auto &sn1 = surfel_graph->add_node(s1);
  const auto &sn2 = surfel_graph->add_node(s2);
  SurfelGraphEdge edge{0.0};
  edge.set_k_low(1);
  edge.set_k_high(3);
  edge.set_t_low(1,2);
  edge.set_t_high(3,4);
  surfel_graph->add_edge(sn1, sn2, edge);
}

void TestSurfelIO::TearDown() {
  delete m_surfel_builder;
}

void TestSurfelGraph::SetUp() {
  using namespace std;

  std::default_random_engine rng{123};
  m_surfel_builder = new SurfelBuilder(rng);
  surfel_graph = make_shared<SurfelGraph>();
  Eigen::Matrix3f transform;
  transform << 1.1f, 2.2f, 3.3f,
      4.4f, 5.5f, 6.6f,
      7.7f, 8.8f, 9.9f;

  m_surfel_builder
      ->reset()
      ->with_id("a")
      ->with_frame({
                       {1, 1, 1}, // pif
                       1.1f, // depth
                       transform,
                       {1.1f, 2.2f, 3.3f}, // norm
                       {1.1f, 2.2f, 3.3f} //pos
                   }
      )
      ->with_tangent(1.0f, 0.0f, 0.0f)
      ->with_reference_lattice_offset(0.15f, 0.25f);
  const auto s1 = make_shared<Surfel>(m_surfel_builder->build());
  s1->set_rosy_smoothness(0);
  s1->set_posy_smoothness(22.9);

  m_surfel_builder
      ->reset()
      ->with_id("b")
      ->with_frame({
                       {1, 1, 1}, // pif
                       1.1f, // depth
                       transform,
                       {1.1f, 2.2f, 3.3f}, // norm
                       {1.1f, 2.2f, 3.3f} //pos
                   }
      )
      ->with_tangent(
          M_SQRT1_2, 0, M_SQRT1_2)
      ->with_reference_lattice_offset(0.35f, 0.45f);
  const auto s2 = make_shared<Surfel>(m_surfel_builder->build());

  s2->set_rosy_smoothness(0);
  s2->set_posy_smoothness(99.2);
  const auto &sn1 = surfel_graph->add_node(s1);
  const auto &sn2 = surfel_graph->add_node(s2);
  surfel_graph->add_edge(sn1, sn2, SurfelGraphEdge{1.0});
}

void TestSurfelGraph::TearDown() {
  delete m_surfel_builder;
}

bool compare_files(const std::string &p1, const std::string &p2) {
  using namespace std;

  ifstream f1(p1, ifstream::binary | ifstream::ate);
  ifstream f2(p2, ifstream::binary | ifstream::ate);

  if (f1.fail()) {
    spdlog::error("Error reading file {:s}", p1);
    return false; //file problem
  }

  if (f2.fail()) {
    spdlog::error("Error reading file {:s}", p2);
    return false; //file problem
  }

  if (f1.tellg() != f2.tellg()) {
    spdlog::error("File are different sizes {:s}: {:d}, {:s}:{:d}", p1, f1.tellg(), p2, f2.tellg());
    return false; //size mismatch
  }

  //seek back to beginning and use std::equal to compare contents
  f1.seekg(0, ifstream::beg);
  f2.seekg(0, ifstream::beg);
  return equal(istreambuf_iterator<char>(f1.rdbuf()),
               istreambuf_iterator<char>(),
               istreambuf_iterator<char>(f2.rdbuf()));
}

void
expect_graphs_equal(const SurfelGraph &graph1,
                    const SurfelGraph &graph2) {
  EXPECT_EQ(graph1.num_nodes(), graph2.num_nodes());
  EXPECT_EQ(graph1.num_edges(), graph2.num_edges());
}

void
expect_edges_equal(SurfelGraph::Edge &edge1,
                   SurfelGraph::Edge &edge2) {
  EXPECT_EQ(edge1.data()->weight(), edge2.data()->weight());
  EXPECT_EQ(edge1.data()->k_low(), edge2.data()->k_low());
  EXPECT_EQ(edge1.data()->k_high(), edge2.data()->k_high());

  EXPECT_EQ(edge1.data()->t_low()[0], edge2.data()->t_low()[0]);
  EXPECT_EQ(edge1.data()->t_high()[0], edge2.data()->t_high()[0]);
  EXPECT_EQ(edge1.data()->t_low()[1], edge2.data()->t_low()[1]);
  EXPECT_EQ(edge1.data()->t_high()[1], edge2.data()->t_high()[1]);
}

void
expect_edges_equal(const std::vector<SurfelGraph::Edge> &edges1, const std::vector<SurfelGraph::Edge> &edges2) {
  using namespace std;

  // We need to index them via a key to compare
  EXPECT_EQ(edges1.size(), edges2.size());

  map<pair<string, string>, SurfelGraph::Edge> map_edges_1;
  for (auto &edge: edges1) {
    string from = edge.from()->data()->id();
    string to = edge.to()->data()->id();
    pair<string, string> key = make_pair<>(from, to);
    map_edges_1.insert({key, edge});
  }
  for (auto edge: edges2) {
    string from = edge.from()->data()->id();
    string to = edge.to()->data()->id();
    pair<string, string> key = make_pair<>(from, to);
    SurfelGraph::Edge other_edge = map_edges_1.at(key);
    expect_edges_equal(edge, other_edge);
  }
}

void
expect_nodes_equal(const SurfelGraphNodePtr &node1,
                   const SurfelGraphNodePtr &node2,
                   bool exclude_smoothness = false) {
  if (!exclude_smoothness) {
    EXPECT_EQ(node1->data()->rosy_smoothness(), node2->data()->rosy_smoothness());
    EXPECT_EQ(node1->data()->posy_smoothness(), node2->data()->posy_smoothness());
  }
  EXPECT_EQ(node1->data()->id(), node2->data()->id());
}

void
expect_node_vectors_equal(
    std::vector<const SurfelGraphNodePtr> &nodes1,
    std::vector<const SurfelGraphNodePtr> &nodes2,
    bool exclude_smoothness = false
) {
  using namespace std;
  EXPECT_EQ(nodes1.size(), nodes2.size());

  map<string, SurfelGraphNodePtr> map_of_nodes;
  for (auto &node: nodes1) {
    map_of_nodes.insert(make_pair(node->data()->id(), node));
  }

  for (auto &node: nodes2) {
    auto other_node = map_of_nodes.at(node->data()->id());
    expect_nodes_equal(node, other_node, exclude_smoothness);
  }
}

void
expect_graphs_equal(const SurfelGraphPtr &graph1,
                    const SurfelGraphPtr &graph2,
                    bool exclude_smoothness = false,
                    bool exclude_edges = false) {
  if (!exclude_edges) {
    auto g1_edges = graph1->edges();
    auto g2_edges = graph2->edges();
    expect_edges_equal(g1_edges, g2_edges);
  }

  std::vector<const SurfelGraphNodePtr> g1_nodes = graph1->nodes();
  std::vector<const SurfelGraphNodePtr> g2_nodes = graph2->nodes();
  expect_node_vectors_equal(g1_nodes, g2_nodes, exclude_smoothness);
}

// Load with no flags. Expect no smoothness or edges
TEST_F(TestSurfelIO, LoadFromTestFileDefault) {
  std::string gold_file_name = "surfel_test_data/gold_graph.bin";
  unsigned short flags = 0;
  auto loaded_graph = load_surfel_graph_from_file(gold_file_name, flags, m_random_engine);
  EXPECT_EQ((flags & FLAG_SMOOTHNESS), 0);
  EXPECT_EQ((flags & FLAG_EDGES), 0);
  expect_graphs_equal(surfel_graph, loaded_graph, true, true);
}

TEST_F(TestSurfelIO, LoadFromTestFileWithEdges) {
  std::string gold_file_name = "surfel_test_data/gold_graph_edges.bin";
  unsigned short flags = 0;
  auto loaded_graph = load_surfel_graph_from_file(gold_file_name, flags, m_random_engine);
  EXPECT_EQ((flags & FLAG_SMOOTHNESS), 0);
  EXPECT_EQ((flags & FLAG_EDGES), FLAG_EDGES);
  expect_graphs_equal(surfel_graph, loaded_graph, true, false);
}

TEST_F(TestSurfelIO, LoadFromTestFileWithSmoothness) {
  std::string gold_file_name = "surfel_test_data/gold_graph_smooth.bin";
  unsigned short flags = 0;
  auto loaded_graph = load_surfel_graph_from_file(gold_file_name, flags, m_random_engine);
  EXPECT_EQ((flags & FLAG_SMOOTHNESS), FLAG_SMOOTHNESS);
  EXPECT_EQ((flags & FLAG_EDGES), 0);
  expect_graphs_equal(surfel_graph, loaded_graph, false, true);
}

TEST_F(TestSurfelIO, SaveToTestFile) {
  std::string gold_file_name = "surfel_test_data/gold_graph.bin";
  std::string tmp_file_name = tmpnam(nullptr);
  save_surfel_graph_to_file(tmp_file_name, surfel_graph);

  // Open and compare to gold file
  EXPECT_TRUE(compare_files(tmp_file_name, gold_file_name));
}

TEST_F(TestSurfelIO, SaveLoadRoundTripDefault) {
  std::string tmp_file_name = std::tmpnam(nullptr);
  // Doesn't save edges or smoothnesss
  save_surfel_graph_to_file(tmp_file_name, surfel_graph);

  unsigned short flags = 0;
  auto loaded_graph = load_surfel_graph_from_file(tmp_file_name, flags, m_random_engine);
  EXPECT_EQ((flags & FLAG_SMOOTHNESS), 0);
  EXPECT_EQ((flags & FLAG_EDGES), 0);

  // Exclude smoothness as we don't persist it.
  expect_graphs_equal(surfel_graph, loaded_graph, true, true);
}

TEST_F(TestSurfelIO, SaveLoadRoundTripWithSmoothness) {
  std::string tmp_file_name = std::tmpnam(nullptr);
  // Doesn't save edges
  save_surfel_graph_to_file(tmp_file_name, surfel_graph, true);

  unsigned short flags = 0;
  auto loaded_graph = load_surfel_graph_from_file(tmp_file_name, flags, m_random_engine);
  EXPECT_EQ((flags & FLAG_SMOOTHNESS), FLAG_SMOOTHNESS);
  EXPECT_EQ((flags & FLAG_EDGES), 0);

  // Exclude smoothness as we don't persist it.
  expect_graphs_equal(surfel_graph, loaded_graph, false, true);
}

TEST_F(TestSurfelIO, SaveLoadRoundTripWithSmoothnessAndEdges) {
  std::string tmp_file_name = std::tmpnam(nullptr);
  save_surfel_graph_to_file(tmp_file_name, surfel_graph, true, true);

  unsigned short flags = 0;
  auto loaded_graph = load_surfel_graph_from_file(tmp_file_name, flags, m_random_engine);
  EXPECT_EQ((flags & FLAG_SMOOTHNESS), FLAG_SMOOTHNESS);
  EXPECT_EQ((flags & FLAG_EDGES), FLAG_EDGES);

  // Exclude smoothness as we don't persist it.
  expect_graphs_equal(surfel_graph, loaded_graph);
}

TEST_F(TestSurfelGraph, k_is_preserved_when_accessed_directly) {
  auto nodes = surfel_graph->nodes(); // Two nodes have an edge between them
  auto edge = surfel_graph->edge(nodes[0], nodes[1]);

  edge->set_k_low(32);
  edge->set_k_high(19);
  auto k_first = edge->k_low();
  auto k_second = edge->k_high();

  EXPECT_EQ(32, k_first);
  EXPECT_EQ(19, k_second);

  // retrieve the edge the other way around
  edge = surfel_graph->edge(nodes[1], nodes[0]);
  auto k_first_next = edge->k_low();
  auto k_second_next = edge->k_high();

  EXPECT_EQ(k_first, k_first_next);
  EXPECT_EQ(k_second, k_second_next);
}

TEST_F(TestSurfelGraph, k_is_switched_based_on_node_order_when_accessed_indirectly) {
  auto nodes = surfel_graph->nodes(); // Two nodes have an edge between them

  set_k(surfel_graph, nodes[0], 32, nodes[1], 19);
  auto ks = get_k(surfel_graph, nodes[0], nodes[1]);
  EXPECT_EQ(32, ks.first);
  EXPECT_EQ(19, ks.second);

  auto switched_ks = get_k(surfel_graph, nodes[1], nodes[0]);
  EXPECT_EQ(19, switched_ks.first);
  EXPECT_EQ(32, switched_ks.second);
}
