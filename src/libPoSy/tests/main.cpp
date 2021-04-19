/**
 * All tests
 */

#include <gtest/gtest.h>
#include <spdlog/cfg/env.h>

/**
 * Run all tests
 */ 
int main(int argc, char **argv) {
    spdlog::cfg::load_env_levels();
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}