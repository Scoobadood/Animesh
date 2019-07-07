//
// Created by Dave Durbin on 2019-07-07.
//

#include "DepthMap/DepthMapPyramid.h"

DepthMapPyramid::DepthMapPyramid(const DepthMap& depth_map) {
    depth_maps.push_back(depth_map);
}

/**
 * Downsample a Depth Map.
 * Generate a new depth map with half the edge length of the source one.
 * New depths are computed as the mean of the
 * @param source_map
 * @return
 */
DepthMap
DepthMapPyramid::down_sample(const DepthMap& source_map) {
    DepthMap dm{};
    for( int row = 0; row < source_map.rows(); ++row) {

    }
}

void DepthMapPyramid::set_num_levels(int num_levels) {
    using namespace std;

    if( num_levels < 1 ) {
        throw runtime_error("Can't have less than one level for a DepthMapPyramid");
    }
    if( depth_maps.size() > num_levels) {
        depth_maps.erase( depth_maps.begin() + num_levels, depth_maps.end());
        return;
    }

    while( depth_maps.size() < num_levels) {
        depth_maps.push_back(down_sample(depth_maps.back()));
    }
}
