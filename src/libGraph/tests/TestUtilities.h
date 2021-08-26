//
// Created by Dave Durbin (Old) on 18/8/21.
//

#ifndef ANIMESH_LIBGRAPH_TESTS_TESTUTILITIES_H
#define ANIMESH_LIBGRAPH_TESTS_TESTUTILITIES_H

#include "gtest/gtest.h"
#include <regex>

#define EXPECT_THROW_WITH_MESSAGE(stmt, etype, whatstring) \
      EXPECT_THROW(                                        \
        try { stmt; }                                      \
        catch (const etype& ex) {                          \
          std::regex r{whatstring};                        \
          std::string s{ex.what()};                        \
          bool matches = regex_match(s, r);                \
          EXPECT_TRUE(matches);                            \
          throw;                                           \
          } , etype)

std::string node_merge_function(const std::string &n1, const std::string &n2);
float edge_merge_function(const float &f1, const float& f2);

#endif //ANIMESH_LIBGRAPH_TESTS_TESTUTILITIES_H
