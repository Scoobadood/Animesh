#include "TestObjFileParser.h"
#include <vector>

void TestObjFileParser::TearDown( ) {}
void TestObjFileParser::SetUp( ) {}

TEST_F(TestObjFileParser, ParseSphereHasCorrectNodeCount) {
  using namespace std;
  using namespace animesh;
  pair<vector<PointNormal::Ptr>, multimap<size_t, size_t>> results = parser.parse_file("../data/sphere10x10/sphere10x10.obj", true);

  size_t expected_nodes = 10 /* num rows */ * (10 /* num segments */ -1) + 2;
  EXPECT_EQ(expected_nodes, results.first.size());
}

// Poles of sphere should have 10 neighbours.
TEST_F(TestObjFileParser, ParseSphereTwoNodesHave10Neighbours) {
  using namespace std;
  using namespace animesh;
  pair<vector<PointNormal::Ptr>, multimap<size_t, size_t>> results = parser.parse_file("../data/sphere10x10/sphere10x10.obj", true);

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
  pair<vector<PointNormal::Ptr>, multimap<size_t, size_t>> results = parser.parse_file("../data/sphere10x10/sphere10x10.obj", true);

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

TEST_F(TestObjFileParser, ParseCubeHasCorrectNormals) {
  using namespace std;
  using namespace Eigen;
  using namespace animesh;

  pair<vector<Vector3f>, vector<pair<vector<size_t>,Vector3f>>> results = parser.parse_file_raw_with_normals("../data/Cube1x1/cube.obj");

  EXPECT_EQ(8, results.first.size());
  EXPECT_EQ(6, results.second.size());
  EXPECT_EQ(-1, results.second[0].second.y());
  EXPECT_EQ(1, results.second[1].second.y());
  EXPECT_EQ(1, results.second[2].second.x());
  EXPECT_EQ(1, results.second[3].second.z());
  EXPECT_EQ(-1, results.second[4].second.x());
  EXPECT_EQ(-1, results.second[5].second.z());
}
