#include "TestSurfel.h"
#include <Surfel/Surfel.h>
#include <Surfel/FrameData.h>
#include <Surfel/Surfel_IO.h>
#include <Surfel/SurfelGraph.h>
#include <Eigen/Core>
#include <gtest/gtest.h>
#include <string>
#include <vector>
#include <memory>
#include <fstream>

void TestSurfel::SetUp() {}

void TestSurfel::TearDown() {}

void TestSurfelIO::SetUp() {
    using namespace std;

    const auto s1 = make_shared<Surfel>("a",
                                        std::vector<FrameData>{},
                                        Eigen::Vector3f{1.0f, 2.0f, 3.0f},
                                        Eigen::Vector2f{1.5f, 2.5f}
    );
    const auto s2 = make_shared<Surfel>("b",
                                        std::vector<FrameData>{},
                                        Eigen::Vector3f{5.0f, 6.0f, 7.0f},
                                        Eigen::Vector2f{5.5f, 6.5f}
    );
    const auto &sn1 = surfel_graph.add_node(s1);
    const auto &sn2 = surfel_graph.add_node(s2);
    surfel_graph.add_edge(sn1, sn2, 1.0);
}

void TestSurfelIO::TearDown() {}

bool compare_files(const std::string &p1, const std::string &p2) {
    using namespace std;

    ifstream f1(p1, ifstream::binary | ifstream::ate);
    ifstream f2(p2, ifstream::binary | ifstream::ate);

    if (f1.fail() ) {
        spdlog::error("Error reading file {:s}", p1);
        return false; //file problem
    }

    if (f2.fail() ) {
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

TEST_F(TestSurfelIO, LoadFromTestFile) {
    std::string gold_file_name = "surfel_test_data/gold_graph.bin";
    auto loaded_graph = load_surfel_graph_from_file(gold_file_name);
    expect_graphs_equal(loaded_graph, surfel_graph);
}

TEST_F(TestSurfelIO, SaveToTestFile) {
    std::string gold_file_name = "surfel_test_data/gold_graph.bin";
    std::string tmp_file_name = std::tmpnam(nullptr);
    save_surfel_graph_to_file(tmp_file_name, surfel_graph);

    // Open and compare to gold file
    EXPECT_TRUE(compare_files(tmp_file_name, gold_file_name));
}

TEST_F(TestSurfelIO, SaveLoadRoundTrip) {
    std::string tmp_file_name = std::tmpnam(nullptr);
    save_surfel_graph_to_file(tmp_file_name, surfel_graph);

    auto loaded_graph = load_surfel_graph_from_file(tmp_file_name);

    expect_graphs_equal(loaded_graph, surfel_graph);
}

