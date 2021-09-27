//
// Created by Dave Durbin (Old) on 26/9/21.
//

#pragma once

#include <GeomFileUtils/ObjFileParser.h>
#include "gtest/gtest.h"

class TestPlyFileParser : public ::testing::Test {
public:
  animesh::ObjFileParser parser;

  void SetUp( ) override;
  void TearDown( ) override;
};
