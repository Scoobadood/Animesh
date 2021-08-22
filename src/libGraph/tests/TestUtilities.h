//
// Created by Dave Durbin (Old) on 18/8/21.
//

#ifndef ANIMESH_LIBGRAPH_TESTS_TESTUTILITIES_H
#define ANIMESH_LIBGRAPH_TESTS_TESTUTILITIES_H

#include "gtest/gtest.h"

#define EXPECT_THROW_WITH_MESSAGE(stmt, etype, whatstring) EXPECT_THROW( \
        try { \
            stmt; \
        } catch (const etype& ex) { \
            EXPECT_EQ(std::string(ex.what()), whatstring); \
            throw; \
        } \
    , etype)


#endif //ANIMESH_LIBGRAPH_TESTS_TESTUTILITIES_H
