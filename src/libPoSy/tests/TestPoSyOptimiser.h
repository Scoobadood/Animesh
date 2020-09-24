//
// Created by Dave Durbin on 18/5/20.
//

#ifndef ANIMESH_TESTPOSYOPTIMISER_H
#define ANIMESH_TESTPOSYOPTIMISER_H


#include <gtest/gtest.h>
#include <Properties/Properties.h>

class TestPoSyOptimiser : public ::testing::Test {
public:
    void SetUp( );
    void TearDown();
    Properties m_properties;
};


#endif //ANIMESH_TESTPOSYOPTIMISER_H
