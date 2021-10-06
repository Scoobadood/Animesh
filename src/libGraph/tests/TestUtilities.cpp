//
// Created by Dave Durbin (Old) on 23/8/21.
//

#include "TestUtilities.h"

std::string node_merge_function(const std::string &n1,
                                const float w1,
                                const std::string &n2,
                                const float w2) {
  return "(" + n1 + "+" + n2 + ")";
}
float edge_merge_function(const float & f1, const float w1, const float& f2, const float w2) {
  return (w1 * f1) + (w2 * f2);
}
