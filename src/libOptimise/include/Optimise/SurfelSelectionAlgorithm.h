//
// Created by Dave Durbin (Old) on 12/4/21.
//

#pragma once

#include <string>
#include <map>

enum SurfelSelectionAlgorithm {
    SSA_SELECT_ALL_IN_RANDOM_ORDER,
    SSA_SELECT_WORST_100,
    SSA_SELECT_WORST_PERCENTAGE
};

const static std::map<std::string, SurfelSelectionAlgorithm> map{
        {"select-all-in-random-order", SSA_SELECT_ALL_IN_RANDOM_ORDER},
        {"select-worst-100", SSA_SELECT_WORST_100},
        {"select-worst-percentage", SSA_SELECT_WORST_PERCENTAGE},
};

static SurfelSelectionAlgorithm with_name( const std::string& algorithm_name ) {
    const auto it = map.find(algorithm_name);
    if( it != map.end() ) {
        return it->second;
    }
    throw std::runtime_error("Unknown surfel selection algorithm " + algorithm_name);
}