//
// Created by Dave Durbin (Old) on 24/5/21.
//


#include "TestGraphNodeSimplifier.h"
#include <Graph/GraphNodeSimplifier.h>

std::string
concat_nodes(const std::vector<TestGraphNodeSimplifier::GraphNodePtr> &nodes) {
    std::string s;
    for (const auto &n : nodes) {
        s += n->data();
    }
    return s;
}

float
concat_edges(const std::vector<float> &edge_data) {
    return 1.0f;
}

void TestGraphNodeSimplifier::SetUp() {
    using namespace animesh;
    using namespace std;
    graph_ptr = make_shared<Graph<string, float>>(false);

    m_node_a = graph_ptr->add_node("a");
    m_node_b = graph_ptr->add_node("b");
    m_node_c = graph_ptr->add_node("c");
    m_node_d = graph_ptr->add_node("d");
    m_node_e = graph_ptr->add_node("e");
    m_node_x = graph_ptr->add_node("x");

    graph_ptr->add_edge(m_node_a, m_node_b, 1.0f);
    graph_ptr->add_edge(m_node_b, m_node_c, 1.0f);
    graph_ptr->add_edge(m_node_c, m_node_d, 1.0f);
    graph_ptr->add_edge(m_node_d, m_node_e, 1.0f);

    m_simplifier = new GraphNodeSimplifier<std::string, float>(concat_nodes, concat_edges);
}

void TestGraphNodeSimplifier::TearDown() {
    graph_ptr = nullptr;
}

TEST_F(TestGraphNodeSimplifier, NeighboursOfAMiddleNodeIsCorrect) {
    using namespace std;
    using namespace animesh;

    std::vector<GraphNodePtr> nodes{m_node_c};

    auto n2 = m_simplifier->neighbours_of(graph_ptr, nodes);

    ASSERT_EQ(n2.size(), 2) << "Expected only 2 neighbours of node_c";
    auto first = *std::next(n2.begin(), 0);
    auto second = *std::next(n2.begin(), 1);
    ASSERT_TRUE((first->data() == "b" && second->data() == "d") ||
                (first->data() == "d" && second->data() == "b")) << "Expected neighbours to be 'b' and 'd'";
}

TEST_F(TestGraphNodeSimplifier, NeighboursOfEmptyListIsEmpty) {
    using namespace std;
    using namespace animesh;

    std::vector<GraphNodePtr> nodes{};
    auto n2 = m_simplifier->neighbours_of(graph_ptr, nodes);

    ASSERT_EQ(n2.size(), 0) << "Not expecting neighbours of empty list";
}

TEST_F(TestGraphNodeSimplifier, NeighboursOfMultipleNodesIsCorrect) {
    using namespace std;
    using namespace animesh;

    std::vector<GraphNodePtr> nodes{m_node_b, m_node_c};
    auto n2 = m_simplifier->neighbours_of(graph_ptr, nodes);

    ASSERT_EQ(n2.size(), 2) << "Expected only 2 neighbours of node_b and node_c";
    auto first = *std::next(n2.begin(), 0);
    auto second = *std::next(n2.begin(), 1);
    ASSERT_TRUE((first->data() == "a" && second->data() == "d") ||
                (first->data() == "d" && second->data() == "a")) << "Expected neighbours to be 'b' and 'd'";
}

// Don;t double count shared nodes
TEST_F(TestGraphNodeSimplifier, NeighboursOfSpacedNodesIsCorrect) {
    using namespace std;
    using namespace animesh;

    std::vector<GraphNodePtr> nodes{m_node_b, m_node_d};
    auto n2 = m_simplifier->neighbours_of(graph_ptr, nodes);

    ASSERT_EQ(n2.size(), 3) << "Expected only 3 neighbours of node_b and node_d";
    bool got_a, got_c, got_e;
    got_a = got_c = got_e = false;
    for (const auto &d : n2) {
        if (d->data() == "a") {
            got_a = true;
        } else if (d->data() == "c") {
            got_c = true;
        } else if (d->data() == "e") {
            got_e = true;
        } else {
            FAIL() << "Found unexpected data " << d->data();
        }
    }

    ASSERT_TRUE(got_a && got_c && got_e) << "Missing one of A, C or E";
}

// One neighbour fo a leaf
TEST_F(TestGraphNodeSimplifier, NeighboursOfALeafNodeIsCorrect) {
    using namespace std;
    using namespace animesh;

    std::vector<GraphNodePtr> nodes{m_node_a};

    auto n2 = m_simplifier->neighbours_of(graph_ptr, nodes);

    ASSERT_EQ(n2.size(), 1) << "Expected only 1 neighbour of node_a";
    ASSERT_EQ((*(n2.begin()))->data(), "b") << "Neighbour should be 'b'";
}

// No neighbours of an unconnected node.
TEST_F(TestGraphNodeSimplifier, NeighboursOfAnOrphanIsCorrect) {
    using namespace std;
    using namespace animesh;

    std::vector<GraphNodePtr> nodes{m_node_x};

    auto n2 = m_simplifier->neighbours_of(graph_ptr, nodes);

    ASSERT_EQ(n2.size(), 0) << "Expected no neighbours";
}

TEST_F(TestGraphNodeSimplifier, SecondNeighboursOfALeafIsCorrect) {
    using namespace std;
    using namespace animesh;

    auto n2 = m_simplifier->second_neighbours_of(graph_ptr, m_node_a);

    ASSERT_EQ(n2.size(), 1) << "Expected one neighbours";
    ASSERT_EQ((*(n2.begin()))->data(), "c") << "2nd Neighbour should be 'c'";
}

TEST_F(TestGraphNodeSimplifier, SecondNeighboursOfAnOrphanIsEmpty) {
    using namespace std;
    using namespace animesh;

    auto n2 = m_simplifier->second_neighbours_of(graph_ptr, m_node_x);

    ASSERT_EQ(n2.size(), 0) << "Expected no neighbours";
}

TEST_F(TestGraphNodeSimplifier, SecondNeighboursOfANearLeafIsCorrect) {
    using namespace std;
    using namespace animesh;

    auto n2 = m_simplifier->second_neighbours_of(graph_ptr, m_node_b);

    ASSERT_EQ(n2.size(), 1) << "Expected one neighbours";
    ASSERT_EQ((*(n2.begin()))->data(), "d") << "2nd Neighbour should be 'd'";
}

TEST_F(TestGraphNodeSimplifier, SecondNeighboursOfAMiddleNodeIsCorrect) {
    using namespace std;
    using namespace animesh;

    auto n2 = m_simplifier->second_neighbours_of(graph_ptr, m_node_c);

    ASSERT_EQ(n2.size(), 2) << "Expected two neighbours";
    auto first = *std::next(n2.begin(), 0);
    auto second = *std::next(n2.begin(), 1);
    ASSERT_TRUE((first->data() == "a" && second->data() == "e") ||
                (first->data() == "e" && second->data() == "a")) << "Expected neighbours to be 'a' and 'e'";
}

void
assertContainsInAnyOrder(
        const std::vector<const std::shared_ptr<TestGraphNodeSimplifier::GraphNode>> & nodes,
        const std::vector<std::string> &expected) {
    bool *found = new bool[expected.size()];
    for (int i = 0; i < expected.size(); ++i) {
        found[i] = false;
    }
    for (const auto &n : nodes) {
        for (int i = 0; i < expected.size(); ++i) {
            if (n->data() == expected[i]) {
                found[i] = true;
            }
        }
    }
    for (int i = 0; i < expected.size(); ++i) {
        if (!found[i]) {
            FAIL() << "Didn't find " << expected[i] << " in results";
        }
    }

    delete[] found;
}

TEST_F(TestGraphNodeSimplifier, CollapseMiddleNode) {
    using namespace std;
    using namespace animesh;

    vector<const GraphNodePtr> removed;
    m_simplifier->collapse_node(graph_ptr, m_node_c, removed);

    ASSERT_EQ(graph_ptr->num_nodes(), 4);
    ASSERT_EQ(graph_ptr->num_edges(), 2); // ab, de

    std::vector<std::string> expected{"a", "e", "x", "dbc"};
    assertContainsInAnyOrder(graph_ptr->nodes(), expected);
    assertContainsInAnyOrder(removed, vector<string>{"b", "c", "d"});

    ASSERT_EQ(removed.size(), 3);
}

TEST_F(TestGraphNodeSimplifier, CollapseEndNode) {
    using namespace std;
    using namespace animesh;

    vector<const GraphNodePtr> removed;
    m_simplifier->collapse_node(graph_ptr, m_node_a, removed);

    ASSERT_EQ(graph_ptr->num_nodes(), 5);
    ASSERT_EQ(graph_ptr->num_edges(), 3); // bc,cd,de,cb,dc,ed

    std::vector<std::string> expected{"c", "d", "e", "x", "ba"};
    assertContainsInAnyOrder(graph_ptr->nodes(), expected);
    assertContainsInAnyOrder(removed, vector<string>{"a", "b"});

    ASSERT_EQ(removed.size(), 2);
}

TEST_F(TestGraphNodeSimplifier, CollapseOrphanNode) {
    using namespace std;
    using namespace animesh;

    vector<const GraphNodePtr> removed;
    m_simplifier->collapse_node(graph_ptr, m_node_x, removed);

    ASSERT_EQ(graph_ptr->num_nodes(), 6);
    ASSERT_EQ(graph_ptr->num_edges(), 4); // ab,bc,cd,de,ed,dc,cb,ba

    std::vector<std::string> expected{"a", "b", "c", "d", "e", "x"};
    assertContainsInAnyOrder(graph_ptr->nodes(), expected);
    ASSERT_EQ(removed.size(), 1);
}

