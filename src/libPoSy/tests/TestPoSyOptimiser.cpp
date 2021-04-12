//
// Created by Dave Durbin on 18/5/20.
//

#include <PoSy/PoSyOptimiser.h>
#include <Surfel/Surfel.h>
#include "TestPoSyOptimiser.h"
#include <memory>
#include <map>

void TestPoSyOptimiser::SetUp() {
    using namespace std;

    m_properties = Properties{
            map<string, string>{
                    {"rho",                        "1.5"},
                    {"convergence-threshold",      "0.01"},
                    {"surfel-selection-algorithm", "select-all-in-random-order"}
            }
    };
}

SurfelGraphPtr
TestPoSyOptimiser::makeTestGraph() {
    SurfelGraphPtr sg = std::make_shared<SurfelGraph>();
    auto s1 = std::make_shared<Surfel>(Surfel("s1",
                                              {
                                                      {
                                                              {0, 0, 0},
                                                              10.0f,
                                                              Eigen::Matrix3f::Identity(),
                                                              {0.0f, 1.0f, 0.0f},
                                                              {0.0f, 0.0f, 0.0f}
                                                      }
                                              },
                                              {1.0f, 0.0f, 0.0f},
                                              {0.0f, 0.0f}));

    auto s2 = std::make_shared<Surfel>(Surfel("s2",
                                              {
                                                      {
                                                              {1, 1, 0},
                                                              10.0f,
                                                              Eigen::Matrix3f::Identity(),
                                                              {0.0f, 1.0f, 0.0f},
                                                              {0.5f, 0.5f, 0.0f}
                                                      }
                                              },
                                              {1.0f, 0.0f, 0.0f},
                                              {0.0f, 0.0f}
    ));
    const auto n1 = sg->add_node(s1);
    const auto n2 = sg->add_node(s2);
    sg->add_edge(n1, n2, SurfelGraphEdge{ 1.0f });
    return sg;
}

void TestPoSyOptimiser::TearDown() {}

/* ********************************************************************************
 * *
 * *  Test average rosy vectors
 * *
 * ********************************************************************************/
TEST_F(TestPoSyOptimiser, FailsAssertionOptimisingWhenUnready) {
    using namespace std;
    ::testing::FLAGS_gtest_death_test_style = "threadsafe";
    PoSyOptimiser optimiser{m_properties};

    ASSERT_DEATH(optimiser.optimise_do_one_step(), "(m_state != UNINITIALISED)");
}

TEST_F(TestPoSyOptimiser, IsReadyOnceDataIsSet) {
    PoSyOptimiser optimiser{m_properties};
    SurfelGraphPtr g = std::make_shared<SurfelGraph>(new SurfelGraph());
    optimiser.set_data(g);

    optimiser.optimise_do_one_step();
}

TEST_F(TestPoSyOptimiser, ConvergesInPlane) {
    using namespace std;
    using namespace Eigen;

    map<string, string> props = {
            {"rho", "1.0"}
            , {"convergence-threshold", "1.0"}
            , {"surfel-selection-algorithm", "select-worst-100"}
    };
    Properties p{props};
    PoSyOptimiser optimiser{p};

    auto graph = makeTestGraph();
    optimiser.set_data(graph);
    optimiser.optimise_do_one_step();
}
