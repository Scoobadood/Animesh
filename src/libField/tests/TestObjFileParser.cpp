#include "TestObjFileParser.h"
#include <vector>

void TestObjFileParser::TearDown( ) {}
void TestObjFileParser::SetUp( ) {}

TEST_F(TestObjFileParser, ParseSphereHasCorrectNodeCount) {
  using namespace std;
  using namespace animesh;
  pair<vector<PointNormal::Ptr>, multimap<size_t, size_t>> results = parser.parse_file_with_adjacency("../data/sphere10x10/sphere10x10.obj");

  size_t expected_nodes = 10 /* num rows */ * (10 /* num segments */ -1) + 2;
  EXPECT_EQ(expected_nodes, results.first.size());
}

// Poles of sphere should have 10 neighbours.
TEST_F(TestObjFileParser, ParseSphereTwoNodesHave10Neighbours) {
  using namespace std;
  using namespace animesh;
  pair<vector<PointNormal::Ptr>, multimap<size_t, size_t>> results = parser.parse_file_with_adjacency("../data/sphere10x10/sphere10x10.obj");

  size_t actual_nodes_with_10_neighbours = 0;

  using MMAPIterator = multimap<size_t, size_t>::iterator;
  int num_nodes = results.first.size();
  for( size_t i=0; i<num_nodes; ++i ) {
	   pair<MMAPIterator, MMAPIterator> result = results.second.equal_range(i);
     int count =  distance(result.first, result.second);
     if( count == 10 ) {
       actual_nodes_with_10_neighbours++;
     }
   }
   EXPECT_EQ(2, actual_nodes_with_10_neighbours);
}

// Non-poles of sphere should have 4 neighbours.
TEST_F(TestObjFileParser, ParseSphereNinetyNodesHave4Neighbours) {
  using namespace std;
  using namespace animesh;
  pair<vector<PointNormal::Ptr>, multimap<size_t, size_t>> results = parser.parse_file_with_adjacency("../data/sphere10x10/sphere10x10.obj");

  size_t actual_nodes_with_4_neighbours = 0;

  using MMAPIterator = multimap<size_t, size_t>::iterator;
  int num_nodes = results.first.size();
  for( size_t i=0; i<num_nodes; ++i ) {
	   pair<MMAPIterator, MMAPIterator> result = results.second.equal_range(i);
     int count =  distance(result.first, result.second);
     if( count == 4 ) {
       actual_nodes_with_4_neighbours++;
     }
   }
   EXPECT_EQ(90, actual_nodes_with_4_neighbours);
}
