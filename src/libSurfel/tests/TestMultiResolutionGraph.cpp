//
// Created by Dave Durbin (Old) on 4/10/21.
//

#include "TestMultiResolutionGraph.h"
#include <Surfel/MultiResolutionSurfelGraph.h>
#include <Surfel/SurfelBuilder.h>

void TestMultiResolutionGraph::SetUp() {
  using namespace std;

  // Generate surfel graph
  m_surfel_graph = make_shared<SurfelGraph>();
  std::default_random_engine rng{123};
  auto *sb = new SurfelBuilder(rng);

  auto surfel1 = sb->with_id("s1")
      ->with_reference_lattice_offset(0.5, 0.5)
      ->with_tangent(1, 0, 0)
      ->with_frame({0, 0, 1}, 1.5, {0, 1, 0}, {4, 4, 4})
      ->with_frame({0, 0, 2}, 1.5, {0, 1, 0}, {5, 4, 4})
      ->with_frame({0, 0, 3}, 1.5, {0, 1, 0}, {6, 4, 4})
      ->build();

  auto surfel2 = sb->with_id("s2")
      ->with_reference_lattice_offset(0.9, 0.1)
      ->with_tangent(0, 0, 1)
      ->with_frame({0, 0, 2}, 2.5, {0, 1, 0}, {4, 4, 4})
      ->with_frame({0, 0, 3}, 2.5, {0, 1, 0}, {4, 5, 4})
      ->with_frame({0, 0, 4}, 2.5, {0, 1, 0}, {4, 6, 4})
      ->build();

  auto node1 = m_surfel_graph->add_node(make_shared<Surfel>(surfel1));
  auto node2 = m_surfel_graph->add_node(make_shared<Surfel>(surfel2));
  m_surfel_graph->add_edge(node1, node2, SurfelGraphEdge{1});
}

void TestMultiResolutionGraph::TearDown() {}

TEST_F(TestMultiResolutionGraph, generate_zero_levels_should_fail) {
  std::default_random_engine rng{123};
  MultiResolutionSurfelGraph g{m_surfel_graph, rng};
  g.generate_levels(0);
}

TEST_F(TestMultiResolutionGraph, generate_multi_levels_should_fail) {
  std::default_random_engine rng{123};
  MultiResolutionSurfelGraph g{m_surfel_graph, rng};
  g.generate_levels(2);
}