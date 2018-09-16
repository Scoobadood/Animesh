#include "TestGraphCycles.h"
#include "Graph/Graph.h"
#include <unordered_set>

void TestGraphCycles::TearDown( ) {}

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
	test_graph.add_edge(gn[2], gn[10], 1.0, 0);
	test_graph.add_edge(gn[3], gn[10], 1.0, 0);
	test_graph.add_edge(gn[9], gn[10], 1.0, 0);
	test_graph.add_edge(gn[8], gn[9], 1.0, 0);
	test_graph.add_edge(gn[6], gn[ 7], 1.0, 0);
	test_graph.add_edge(gn[7], gn[ 8], 1.0, 0);
	test_graph.add_edge(gn[8], gn[ 14], 1.0, 0);
	test_graph.add_edge(gn[9], gn[13], 1.0, 0);
	test_graph.add_edge(gn[13], gn[14], 1.0, 0);
	test_graph.add_edge(gn[14], gn[16], 1.0, 0);
	test_graph.add_edge(gn[15], gn[16], 1.0, 0);
	test_graph.add_edge(gn[1], gn[ 15], 1.0, 0);
	test_graph.add_edge(gn[12], gn[13], 1.0, 0);
	test_graph.add_edge(gn[11], gn[2], 1.0, 0);
	test_graph.add_edge(gn[1], gn[11], 1.0, 0);
	test_graph.add_edge(gn[2], gn[11], 1.0, 0);
	test_graph.add_edge(gn[13], gn[15], 1.0, 0);
}

TEST_F(TestGraphCycles, CyclesAreCorrect ) {
	using namespace std;

	unordered_set<vector<size_t>, animesh::vector_hash> cycles = test_graph.cycles( );

	vector<vector<size_t>> expected_cycles =
	{
		{ 1, 2, 11 },
		{ 2, 3, 10},
		{8, 9, 13, 14},
		{13, 14, 15, 16},
		{3, 4, 5, 6, 7, 8, 9, 10}
	};
	/*
		Expected cycles
		[1, 2, 11]
		[2, 3, 10]
		[8, 9, 13, 14]
		[13, 14, 15, 16]
		[3, 4, 5, 6, 7, 8, 9, 10]
	*/
	EXPECT_EQ( expected_cycles.size(), cycles.size());
	for( auto expected_cycle : expected_cycles ) {
		EXPECT_EQ( 1, cycles.count( expected_cycle));
	}
}
