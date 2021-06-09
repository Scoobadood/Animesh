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
                    {"rho",                                "1.5"},
                    {"posy-termination-criteria",          "relative"},
                    {"posy-term-crit-relative-smoothness", "0.01"},
                    {"posy-surfel-selection-algorithm",    "select-all-in-random-order"},
                    {"trace-smoothing",                    "false"},
                    {"posy-randomise-neighbour-order",     "false"},
                    {"diagnose_dodgy_deltas",              "true"}
            }
    };
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