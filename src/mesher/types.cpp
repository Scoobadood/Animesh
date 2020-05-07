//
// Created by Dave Durbin on 7/5/20.
//

#include "types.h"

std::map<std::string, std::reference_wrapper<Surfel>> Surfel::surfel_by_id = []{
    return std::map<std::string, std::reference_wrapper<Surfel>>{};
}();

Surfel& Surfel::surfel_for_id(const std::string& id) {
    auto it = surfel_by_id.find(id);
    if( it != surfel_by_id.end() ) {
        Surfel& ret = it->second;
        return ret;
    }
    std::cerr << "No surfel found for ID " << id << std::endl;
    throw std::runtime_error("Bad surfel id");
}