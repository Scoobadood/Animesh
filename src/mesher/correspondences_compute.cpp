#include <iostream>
#include <string>
#include <regex>
#include <map>

#include "surfel_compute.h"
#include "correspondences_compute.h"
#include <cpd/nonrigid.hpp>


cpd::Matrix
depth_map_to_pointcloud(const Camera& camera, const DepthMap& depth_map) {
    using namespace std;

    struct Point3D {
        float x;
        float y;
        float z;
        Point3D( float x, float y, float z ) : x(x), y(y), z(z) {};
    };

    // Filter out zero depth points
    vector<Point3D> valid_points;
    for( unsigned int r = 0; r < depth_map.rows(); ++r ) {
        for( unsigned int c = 0; c < depth_map.cols(); ++c ) {
            float depth = depth_map.depth_at(r, c);
            if( depth > 0.0f ) {
                // backproject using camera settings
                float x, y, z;
                backproject( camera, c, r, depth, &x, &y, &z);

                valid_points.emplace_back( x, y, z);
            }
        }
    }

    // Convert remaining to a Nx3 matrix
    cpd::Matrix m{ valid_points.size(), 3};
    int row = 0;
    for( const auto& p : valid_points ) {
        m(row,0) = p.x;
        m(row,1) = p.y;
        m(row,2) = p.z;
        row++;
    }
    return m;
}

/**
 * Compute the correspondences between pixels in cleaned depth maps
 * Returns a vector or correspondences where each correspondence
 * is a PixelInFrame
 */
std::vector<std::vector<PixelInFrame>>
compute_correspondences(const std::vector<Camera>& cameras, const std::vector<DepthMap>& depth_maps) {
    using namespace std;

    cout << "Computing correspondences..." << flush;

    vector<vector<PixelInFrame>> correspondences;

    // Convert each frame to a cpd::Matrix
    vector<cpd::Matrix> depth_maps_as_pointclouds;
    int cam_index = 0;
    for( const auto& depth_map : depth_maps) {
        cpd::Matrix pc = depth_map_to_pointcloud(cameras.at(cam_index), depth_map);
        depth_maps_as_pointclouds.push_back(pc);
        cam_index++;
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
