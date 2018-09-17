#include "TestGraphCycles.h"
#include "Graph/Graph.h"
#include <unordered_set>
#include <vector>

void TestGraphCycles::TearDown( ) {}

/*
  Test Graph


    1       2      3      4      5      6
		 o------o------o------o------o------o
     |\   /  \    /                     |
		 | \ /	  \  /                      |
		 |  o      o----------o------o------o 7
     | 11\     10         9      8
	   |    \               |      |
		 |     \           13 |      | 14
		 |   12 o-------------o------o
  15 o-------------------/      /
	    \            16          /
	     \____________o_________/
 */
void TestGraphCycles::SetUp( ) {
	using namespace animesh;

	Graph<size_t, int>::GraphNode * gn[17];
	for ( size_t idx = 1; idx <= 16; ++idx ) {
		gn[idx] = test_graph.add_node( idx );
	}
	test_graph.add_edge(gn[1], gn[2], 1.0, 0);
	test_graph.add_edge(gn[2], gn[3], 1.0, 0);
	test_graph.add_edge(gn[3], gn[4], 1.0, 0);
	test_graph.add_edge(gn[4], gn[5], 1.0, 0);
	test_graph.add_edge(gn[5], gn[6], 1.0, 0);
	test_graph.add_edge(gn[6], gn[7], 1.0, 0);
	test_graph.add_edge(gn[7], gn[8], 1.0, 0);
	test_graph.add_edge(gn[8], gn[9], 1.0, 0);
	test_graph.add_edge(gn[9], gn[10], 1.0, 0);
	test_graph.add_edge(gn[11], gn[12], 1.0, 0);
	test_graph.add_edge(gn[12], gn[13], 1.0, 0);
	test_graph.add_edge(gn[13], gn[14], 1.0, 0);
	test_graph.add_edge(gn[14], gn[16], 1.0, 0);
	test_graph.add_edge(gn[1], gn[11], 1.0, 0);
	test_graph.add_edge(gn[2], gn[11], 1.0, 0);
	test_graph.add_edge(gn[2], gn[10], 1.0, 0);
	test_graph.add_edge(gn[3], gn[10], 1.0, 0);
	test_graph.add_edge(gn[8], gn[14], 1.0, 0);
	test_graph.add_edge(gn[9], gn[13], 1.0, 0);
	test_graph.add_edge(gn[13], gn[15], 1.0, 0);
	test_graph.add_edge(gn[1], gn[15], 1.0, 0);
	test_graph.add_edge(gn[15], gn[16], 1.0, 0);
}

TEST_F(TestGraphCycles, PathEquivalencyWhenSameUndirected) {
	using namespace std;

	animesh::Path p1{ vector<size_t>{ 0, 1, 10 }};
	animesh::Path p2{ vector<size_t>{ 0, 1, 10 }};
	EXPECT_TRUE(p1.is_equivalent_to(p2));
}

TEST_F(TestGraphCycles, PathEquivalencyWhenShiftedUndirected) {
	using namespace std;

	animesh::Path p1{ vector<size_t>{ 0, 1, 10 }};
	animesh::Path p2{ vector<size_t>{ 1, 10, 0 }};
	EXPECT_TRUE(p1.is_equivalent_to(p2));
}

TEST_F(TestGraphCycles, PathEquivalencyWhenOppositeUndirected) {
	using namespace std;

	animesh::Path p1{ vector<size_t>{ 0, 1, 10 }};
	animesh::Path p2{ vector<size_t>{ 0, 10, 1 }};
	EXPECT_TRUE(p1.is_equivalent_to(p2));
}

TEST_F(TestGraphCycles, NoPathEquivalencyWhenRandomUndirected) {
	using namespace std;

	animesh::Path p1{ vector<size_t>{ 0, 1, 2, 3 }};
	animesh::Path p2{ vector<size_t>{ 0, 1, 3, 2 }};
	EXPECT_FALSE(p1.is_equivalent_to(p2));
}

TEST_F(TestGraphCycles, CyclesAreCorrect ) {
	using namespace std;

	unordered_set<animesh::Path> cycles = test_graph.cycles( );

	// Note cycles are index order, not node value order`

	vector<animesh::Path> expected_cycles =
	{
		animesh::Path{ vector<size_t>{ 0, 1, 10 }},
		animesh::Path{ vector<size_t>{ 1, 2, 9}},
		animesh::Path{ vector<size_t>{ 2, 3, 4, 5, 6, 7, 8, 9}},
		animesh::Path{ vector<size_t>{ 7, 13, 12, 8}},
		animesh::Path{ vector<size_t>{ 12, 13, 15, 14}},
		animesh::Path{ vector<size_t>{ 0, 10, 11, 12, 14}},
	};
	EXPECT_EQ( expected_cycles.size(), cycles.size());
	for( auto expected_cycle : expected_cycles ) {
		// Find it
		bool found = false;
		for( auto actual_cycle : cycles) {
			if( actual_cycle.length() != expected_cycle.length() ) {
				continue;
			}
			found = (expected_cycle.is_equivalent_to(actual_cycle));
			if( found ) {
				break;
			}
		}
		EXPECT_EQ( true, found);
	}
}
