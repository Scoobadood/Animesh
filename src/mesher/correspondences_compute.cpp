#include <iostream>
#include <string>
#include <regex>
#include <map>

#include "surfel_compute.h"
#include "correspondences_compute.h"
#include <cpd/nonrigid.hpp>
#include <cpd/gauss_transform_fgt.hpp>

// Given a depth map, extract pixels which have a non-zero depth
static std::vector<Pixel>
depth_map_to_pixels(const DepthMap &depth_map) {
    using namespace std;

    // Filter out zero depth points
    vector<Pixel> valid_pixels;
    for (unsigned int r = 0; r < depth_map.rows(); ++r) {
        for (unsigned int c = 0; c < depth_map.cols(); ++c) {
            float depth = depth_map.depth_at(r, c);
            if (depth > 0.0f) {
                valid_pixels.emplace_back(c, r);
            }
        }
    }
    return valid_pixels;
}


/*
 * Given a camera, a set of N X,Y coordinates and a depth map, genertate camera space 3D coordinates
 * and return as an Nx3 matrix
 */
static cpd::Matrix
pixels_to_pointcloud(const Camera &camera, const std::vector<Pixel> &pixels,
                     const DepthMap &depth_map) {
    using namespace std;

    struct Point3D {
        float x;
        float y;
        float z;

        Point3D(float x, float y, float z) : x(x), y(y), z(z) {};
    };

    // Filter out zero depth points
    vector<Point3D> valid_points;
    for (const auto &pixel : pixels) {
        float depth = depth_map.depth_at(pixel.y, pixel.x);
        // backproject using camera settings
        float x, y, z;
        backproject(camera, pixel.x, pixel.y, depth, &x, &y, &z);
        valid_points.emplace_back(x, y, z);
    }

    // Convert remaining to a Nx3 matrix
    cpd::Matrix m{valid_points.size(), 3};
    int row = 0;
    for (const auto &p : valid_points) {
        m(row, 0) = p.x;
        m(row, 1) = p.y;
        m(row, 2) = p.z;
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
compute_correspondences(const std::vector<Camera> &cameras, const std::vector<DepthMap> &depth_maps) {
    using namespace std;

    cout << "Computing correspondences..." << flush;

    vector<vector<PixelInFrame>> correspondences;

    // TODO: We need to maintain the PiF information as this is used to drive the correspondences
    // but they are compute across the the point clouds thus we need a point in cloud to pif mapping
    // Stripping the points with zero depth before producing the point clouds sounds sane

    // Convert each frame to a cpd::Matrix
    vector<cpd::Matrix> depth_maps_as_pointclouds;
    vector<vector<Pixel>> valid_pixels;
    int cam_index = 0;
    for (const auto &depth_map : depth_maps) {
        vector<Pixel> valid_pixels_in_frame = depth_map_to_pixels(depth_map);
        valid_pixels.push_back(valid_pixels_in_frame);
        cpd::Matrix pc = pixels_to_pointcloud(cameras.at(cam_index), valid_pixels_in_frame, depth_map);
        depth_maps_as_pointclouds.push_back(pc);
        cam_index++;
    }

    /*
     * Create a multimap from correspondence group to points in it
     * Frame = 0
     * CorrIdx = 0
     * Compute correspondences to frame 1
     * For each correspondence
     *    Make src_pif
     *    Make tgt_pif
     *    // One of these is tru
     *    // neither source nor tgt has corr group yet
     *    // Src has group but not target --> add tgt to source group
     *    // Tgt has group but not src --> add src to tgt group
     *    // Neither has group --> Make new group and add both
     *    // Both have same group --> Do nothing
     *    // Both have different groupd --> merge groups
     */
    multimap<unsigned int, PixelInFrame> group_to_pif;
    map<PixelInFrame, unsigned int> pif_to_group;

    // Group IDs begin at 1. 0 is an invalid group
    unsigned int group_idx = 1;

    cpd::Nonrigid nonrigid;
    nonrigid.correspondence(true);
    nonrigid.normalize(false);
    nonrigid.gauss_transform(
            move(
                    unique_ptr<cpd::GaussTransform>(new cpd::GaussTransformFgt())
            )
    );

    for (unsigned int src_frame_idx = 0; src_frame_idx < depth_maps_as_pointclouds.size() - 1; ++src_frame_idx) {
        unsigned int tgt_frame_idx = src_frame_idx + 1;
        cout << "Computing correspondences between frame " << src_frame_idx << " and " << tgt_frame_idx << endl;

        const auto &fixed = depth_maps_as_pointclouds.at(tgt_frame_idx);
        const auto &moving = depth_maps_as_pointclouds.at(src_frame_idx);

        cpd::NonrigidResult result = nonrigid.run(fixed, moving);

        for (unsigned int src_point_idx = 0; src_point_idx < result.correspondence.size(); ++src_point_idx) {
            unsigned int tgt_point_idx = result.correspondence(src_point_idx);
            const Pixel &src_pixel = valid_pixels.at(src_frame_idx).at(src_point_idx);
            const Pixel &tgt_pixel = valid_pixels.at(tgt_frame_idx).at(tgt_point_idx);
            PixelInFrame src_pif{src_pixel, src_frame_idx};
            PixelInFrame tgt_pif{tgt_pixel, tgt_frame_idx};

            auto it = pif_to_group.find(src_pif);
            unsigned int src_group = (it == pif_to_group.end()) ? 0 : it->second;
            it = pif_to_group.find(tgt_pif);
            unsigned int tgt_group = (it == pif_to_group.end()) ? 0 : it->second;

            if ( /* src pif is in a group but target pif is not*/ src_group > 0 && tgt_group == 0) {
                // Add target pif to src group
                group_to_pif.insert(make_pair(src_group, tgt_pif));
                pif_to_group.emplace(tgt_pif, src_group);
            } else if ( /* target pif is in a group but source pif is not */ tgt_group > 0 && src_group == 0) {
                // Add source pif to target group
                group_to_pif.insert(make_pair(tgt_group, src_pif));
                pif_to_group.emplace(src_pif, tgt_group);
            } else if ( /* neither pif is in a group */ src_group == 0 && tgt_group == 0) {
                // Make a new group and add both pifs
                group_to_pif.insert(make_pair(group_idx, src_pif));
                group_to_pif.insert(make_pair(group_idx, tgt_pif));
                pif_to_group.emplace(src_pif, group_idx);
                pif_to_group.emplace(tgt_pif, group_idx);
                group_idx++;
            } else if ( /* src and target pifs are in different groups */ src_group != tgt_group) {
                // TOD Merge the two groups
            } /* implicit else if same group do nothing */
        }
    }
    // Enumerate groups
    for (auto it = group_to_pif.begin(); it != group_to_pif.end();) {
        unsigned int group = it->first;
        cout << "Group : " << group << endl;
        // Advance to next non-duplicate entry.
        int count = 0;
        vector<PixelInFrame> this_group;
        do {
            cout << "[" << it->second.frame << ", " << it->second.pixel.x << ", " << it->second.pixel.y << "] ";
            this_group.push_back(it->second);
            ++it;
            ++count;
        } while (it != group_to_pif.end() && group == it->first);
        correspondences.push_back(this_group);
        cout << " (" << count << " members)" << endl;
    }
    cout << group_to_pif.size() << " correspondence groups found" << endl;

    // Now all PiFs are in map
    return correspondences;
}
