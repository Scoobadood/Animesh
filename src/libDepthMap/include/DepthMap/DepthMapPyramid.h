//
// Created by Dave Durbin on 2019-07-07.
// A DepthMapPyramid is a collection of DepthMaps where there is a base level map and subsequent
// levels are computed by progressively refining the previous level, merging neighbouring pixels
// and generating a new depth for them.
//

#ifndef ANIMESH_DEPTHMAPPYRAMID_H
#define ANIMESH_DEPTHMAPPYRAMID_H


#include <vector>
#include <map>
#include "DepthMap.h"

class DepthMapPyramid {
public:
    explicit DepthMapPyramid(const DepthMap &depth_map);

    void set_num_levels(int num_levels);

private:
    std::vector<DepthMap> depth_maps;

    DepthMap down_sample(const DepthMap &source_map, std::multimap<std::pair<int,int>, std::pair<int,int>>& mapping);

    /**
     * Delete unwanted levels and their mappings
     */
    void
    remove_levels(int from_level);

    /**
     * Downsample the top level to get a new one.
     */
    void
    make_new_level();

    // Maps from a pixel in one tier to a pixel in the next tier
    std::vector<std::multimap<std::pair<int, int>, std::pair<int, int>>> mappings;
};

#endif //ANIMESH_DEPTHMAPPYRAMID_H
