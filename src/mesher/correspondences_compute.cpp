#include <iostream>
#include <string>
#include <regex>
#include <map>

#include "surfel_compute.h"
#include "correspondences_compute.h"
#include <cpd/nonrigid.hpp>


cpd::Matrix
depth_map_to_pointcloud(const DepthMap& depth_map) {
    return cpd::Matrix{};
}

/**
 * Compute the correspondences between pixels in cleaned depth maps
 * Returns a vector or correspondences where each correspondence
 * is a PixelInFrame
 */
std::vector<std::vector<PixelInFrame>>
compute_correspondences(const std::vector<DepthMap>& depth_maps) {
    using namespace std;

    cout << "Computing correspondences..." << flush;

    vector<vector<PixelInFrame>> correspondences;

    // Convert each frame to a cpd::Matrix
    vector<cpd::Matrix> depth_maps_as_pointclouds;
    for( const auto& depth_map : depth_maps) {
        cpd::Matrix pc = depth_map_to_pointcloud(depth_map);
        depth_maps_as_pointclouds.push_back(pc);
    }

    for( int src_frame_idx = 0; src_frame_idx < depth_maps_as_pointclouds.size() - 1; ++src_frame_idx) {
        for( int tgt_frame_idx = src_frame_idx + 1; tgt_frame_idx < depth_maps_as_pointclouds.size(); ++tgt_frame_idx) {
            const auto& fixed = depth_maps_as_pointclouds.at(tgt_frame_idx);
            const auto& moving = depth_maps_as_pointclouds.at(src_frame_idx);
            cpd::NonrigidResult result = cpd::nonrigid(fixed, moving);
        }
    }

    cout << endl;
    return correspondences;
}
