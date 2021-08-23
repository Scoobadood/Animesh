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

#endif //ANIMESH_LIBGRAPH_TESTS_TESTUTILITIES_H
