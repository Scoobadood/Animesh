#include "TestGraphCycles.h"

#include <Graph/Graph.h>
#include <GeomFileUtils/ObjFileParser.h>
#include <unordered_set>
#include <vector>

void TestGraphCycles::TearDown() {}

/*
  Test Graph


         1      2      3      4      5      6
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
animesh::Graph<std::size_t, int> setup_test_graph() {
    using namespace std;

    animesh::Graph<std::size_t, int> test_graph;
    std::vector<shared_ptr<animesh::Graph<std::size_t, int>::GraphNode>> nodes;
    nodes.push_back(nullptr); // Placeholder, not used.
    for (size_t idx = 1; idx <= 16; ++idx) {
        nodes.push_back(test_graph.add_node(idx));
    }
    test_graph.add_edge(nodes.at(1), nodes.at(2), 1.0);
    test_graph.add_edge(nodes.at(2), nodes.at(3), 1.0);
    test_graph.add_edge(nodes.at(3), nodes.at(4), 1.0);
    test_graph.add_edge(nodes.at(4), nodes.at(5), 1.0);
    test_graph.add_edge(nodes.at(5), nodes.at(6), 1.0);
    test_graph.add_edge(nodes.at(6), nodes.at(7), 1.0);
    test_graph.add_edge(nodes.at(7), nodes.at(8), 1.0);
    test_graph.add_edge(nodes.at(8), nodes.at(9), 1.0);
    test_graph.add_edge(nodes.at(9), nodes.at(10), 1.0);
    test_graph.add_edge(nodes.at(11), nodes.at(12), 1.0);
    test_graph.add_edge(nodes.at(12), nodes.at(13), 1.0);
    test_graph.add_edge(nodes.at(13), nodes.at(14), 1.0);
    test_graph.add_edge(nodes.at(14), nodes.at(16), 1.0);
    test_graph.add_edge(nodes.at(1), nodes.at(11), 1.0);
    test_graph.add_edge(nodes.at(2), nodes.at(11), 1.0);
    test_graph.add_edge(nodes.at(2), nodes.at(10), 1.0);
    test_graph.add_edge(nodes.at(3), nodes.at(10), 1.0);
    test_graph.add_edge(nodes.at(8), nodes.at(14), 1.0);
    test_graph.add_edge(nodes.at(9), nodes.at(13), 1.0);
    test_graph.add_edge(nodes.at(13), nodes.at(15), 1.0);
    test_graph.add_edge(nodes.at(1), nodes.at(15), 1.0);
    test_graph.add_edge(nodes.at(15), nodes.at(16), 1.0);
    return test_graph;
}

animesh::Graph<animesh::PointNormal::Ptr, int>
setup_object(const std::string &file_name, bool face_wise) {
    using namespace std;
    using namespace animesh;

    ObjFileParser parser;
    Graph<animesh::PointNormal::Ptr, int> object_graph;

    auto normals_and_adjacency = animesh::ObjFileParser::parse_file(file_name, true, face_wise);

    vector<shared_ptr<Graph<PointNormal::Ptr, int>::GraphNode>> gn2;
    for( const auto& pt : normals_and_adjacency.first) {
        gn2.push_back(object_graph.add_node(pt));
    }
    for( auto e: normals_and_adjacency.second) {
        if( !object_graph.has_edge(gn2[e.first], gn2[e.second])) {
            object_graph.add_edge(gn2[e.first], gn2[e.second], 1.0);
        }
    }
    return object_graph;
}


void TestGraphCycles::SetUp() {
    using namespace animesh;
    using namespace std;

    m_test_graph = setup_test_graph();
}

TEST_F(TestGraphCycles, IdenticalPathsAreEqual) {
    using namespace std;

    animesh::Path<size_t> p1{vector<size_t>{0, 1, 10}};
    animesh::Path<size_t> p2{vector<size_t>{0, 1, 10}};

    EXPECT_EQ(p1, p2);
}

TEST_F(TestGraphCycles, IdenticalPathsAreEquivalent) {
    using namespace std;

    animesh::Path<size_t> p1{vector<size_t>{0, 1, 10}};
    animesh::Path<size_t> p2{vector<size_t>{0, 1, 10}};
    EXPECT_TRUE(p1.is_equivalent_to(p2));
}

TEST_F(TestGraphCycles, DifferentLengthPathsAreNotEquivalent) {
    using namespace std;

    animesh::Path<size_t> p1{vector<size_t>{0, 1, 2, 0}};
    animesh::Path<size_t> p2{vector<size_t>{0, 1, 2, 3, 0}};

    EXPECT_FALSE(p1.is_equivalent_to(p2));
}


TEST_F(TestGraphCycles, ShiftedPathsAreNotEqual) {
    using namespace std;

    animesh::Path<size_t> p1{vector<size_t>{0, 1, 10}};
    animesh::Path<size_t> p2{vector<size_t>{1, 10, 0}};
    EXPECT_FALSE(p1 == p2);
}

TEST_F(TestGraphCycles, ShiftedPathsAreNotEquivalent) {
    using namespace std;

    animesh::Path<size_t> p1{vector<size_t>{0, 1, 10}};
    animesh::Path<size_t> p2{vector<size_t>{1, 10, 0}};
    EXPECT_FALSE(p1.is_equivalent_to(p2));
}

TEST_F(TestGraphCycles, IdenticalCyclesAreEqual) {
    using namespace std;

    animesh::Path<size_t> p1{vector<size_t>{1, 2, 3, 1}};
    animesh::Path<size_t> p2{vector<size_t>{1, 2, 3, 1}};
    EXPECT_EQ(p1, p2);
}

TEST_F(TestGraphCycles, IdenticalCyclesAreEquivalent) {
    using namespace std;

    animesh::Path<size_t> p1{vector<size_t>{1, 2, 3, 1}};
    animesh::Path<size_t> p2{vector<size_t>{1, 2, 3, 1}};
    EXPECT_TRUE(p1.is_equivalent_to(p2));
}

TEST_F(TestGraphCycles, ReversedCyclesAreEquivalent) {
    using namespace std;

    animesh::Path<size_t> p1{vector<size_t>{1, 2, 3, 1}};
    animesh::Path<size_t> p2{vector<size_t>{1, 3, 2, 1}};
    EXPECT_TRUE(p1.is_equivalent_to(p2));
}

TEST_F(TestGraphCycles, OnceShiftedCyclesAreEquivalent) {
    using namespace std;

    animesh::Path<size_t> p1{vector<size_t>{1, 2, 3, 1}};
    animesh::Path<size_t> p2{vector<size_t>{2, 3, 1, 2}};
    EXPECT_TRUE(p1.is_equivalent_to(p2));
}

TEST_F(TestGraphCycles, TwiceShiftedCyclesAreEquivalent) {
    using namespace std;

    animesh::Path<size_t> p1{vector<size_t>{1, 2, 3, 1}};
    animesh::Path<size_t> p2{vector<size_t>{3, 1, 2, 3}};
    EXPECT_TRUE(p1.is_equivalent_to(p2));
}



TEST_F(TestGraphCycles, ReversedOnceShiftedCyclesAreEquivalent) {
    using namespace std;

    animesh::Path<size_t> p1{vector<size_t>{1, 2, 3, 1}};
    animesh::Path<size_t> p2{vector<size_t>{2, 1, 3, 2}};
    EXPECT_TRUE(p1.is_equivalent_to(p2));
}

TEST_F(TestGraphCycles, ReversedTwiceShiftedCyclesAreEquivalent) {
    using namespace std;

    animesh::Path<size_t> p1{vector<size_t>{1, 2, 3, 1}};
    animesh::Path<size_t> p2{vector<size_t>{1, 3, 2, 1}};
    EXPECT_TRUE(p1.is_equivalent_to(p2));
}

TEST_F(TestGraphCycles, SphereCycleCountIs100) {
    // Load the sphere
    animesh::Graph<animesh::PointNormal::Ptr, int> sphere10x10;
    sphere10x10 = setup_object("graph_test_data/sphere10x10.obj", false);

    EXPECT_EQ(100, sphere10x10.cycles().size());
}

TEST_F(TestGraphCycles, SphereFaceCycleCountIs92) {
    // Load the sphere
    animesh::Graph<animesh::PointNormal::Ptr, int> sphere10x10;
    sphere10x10 = setup_object("graph_test_data/sphere10x10.obj", true);

    EXPECT_EQ(92, sphere10x10.cycles().size());
}

TEST_F(TestGraphCycles, SphereFaceCycleCountIs22) {
    // Load the sphere
    animesh::Graph<animesh::PointNormal::Ptr, int> sphere5x5;
    sphere5x5 = setup_object("graph_test_data/sphere.obj", true);

    EXPECT_EQ(22, sphere5x5.cycles().size());
}

// 0,1,2,3,0     0,1,5,4,0
// 0,3,7,4,0     6,2,3,7,6
// 6,2,1,5,6     6,7,4,5,6
TEST_F(TestGraphCycles, CubeCycleCountIs6) {
    animesh::Graph<animesh::PointNormal::Ptr, int> cube = setup_object("graph_test_data/cube.obj", false);

    EXPECT_EQ(6, cube.cycles().size());
}

TEST_F(TestGraphCycles, CubeFaceCycleCountIs8) {
    animesh::Graph<animesh::PointNormal::Ptr, int> cube = setup_object("graph_test_data/cube.obj", true);

    // 8 vertices
    EXPECT_EQ(8, cube.cycles().size());
}

// 289 vertices
// 256 faces
// Should be one cycle per face in node mode
TEST_F(TestGraphCycles, ClothPlaneCycleCountIs256) {
    animesh::Graph<animesh::PointNormal::Ptr, int> cloth = setup_object("graph_test_data/cloth2_1.obj", false);

    // 256 vertices
    auto actual_cycles = cloth.cycles();
    EXPECT_EQ(256, actual_cycles.size());
}

// 16x16 faces
// But each cycle covers 4 faces
// Should be 64 cyles
TEST_F(TestGraphCycles, ClothPlaneFaceCycleCountIs256) {
    animesh::Graph<animesh::PointNormal::Ptr, int> cloth = setup_object("graph_test_data/cloth2_1.obj", true);

    auto actual_cycles = cloth.cycles();
    EXPECT_EQ(64, actual_cycles.size());
}

TEST_F(TestGraphCycles, CyclesAreCorrect) {
    using namespace std;

    // Note cycles are value order`
    vector<animesh::Path<size_t>> expected_cycles = {
            animesh::Path<size_t>{vector<size_t>{1, 2, 11, 1}},
            animesh::Path<size_t>{vector<size_t>{2, 3, 10, 2}},
            animesh::Path<size_t>{vector<size_t>{3, 4, 5, 6, 7, 8, 9, 10, 3}},
            animesh::Path<size_t>{vector<size_t>{8, 14, 13, 9, 8}},
            // Shorter cycles exist from all nodes in this path
            // animesh::Path<size_t>{vector<size_t>{2, 10, 9, 13, 12, 11, 2}},
            animesh::Path<size_t>{vector<size_t>{1, 11, 12, 13, 15, 1}},
            animesh::Path<size_t>{vector<size_t>{13, 14, 16, 15, 13}},
    };

    auto cycles = m_test_graph.cycles();
    EXPECT_EQ(expected_cycles.size(), cycles.size());


    // Convert to size_t paths
    vector<animesh::Path<size_t>> actual_cycles;
    for (const auto &c : cycles) {
        animesh::Path<size_t> p;
        for (int i = 0; i < c.length(); ++i) {
            p.push_back(c[i]->data());
        }
        actual_cycles.push_back(p);
    }

    for (const auto &expected_cycle : expected_cycles) {
        // Find it
        bool found = false;
        for (const auto &actual_cycle : actual_cycles) {
            if (actual_cycle.length() != expected_cycle.length()) {
                continue;
            }
            found = (expected_cycle.is_equivalent_to(actual_cycle));
            if (found) {
                break;
            }
        }
        EXPECT_EQ(true, found);
    }
}
