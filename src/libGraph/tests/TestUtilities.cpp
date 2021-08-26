//
// Created by Dave Durbin (Old) on 23/8/21.
//

#include "TestUtilities.h"

std::string node_merge_function(const std::string &n1, const std::string &n2) {
  return "(" + n1 + "+" + n2 + ")";
}
float edge_merge_function(const float & f1, const float& f2) {
  return f1 + f2;
}
