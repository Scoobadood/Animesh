#include <iostream>
#include <string>
#include <regex>
#include <map>

#include "surfel_compute.h"
#include "correspondences_compute.h"
#include <cpd/nonrigid.hpp>

static std::vector<std::pair<unsigned int, unsigned int>>
depth_map_to_pixels(const DepthMap &depth_map) {
    using namespace std;

    // Filter out zero depth points
    vector<pair<unsigned int, unsigned int>> valid_pixels;
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


static cpd::Matrix
pixels_to_pointcloud(const Camera &camera, const std::vector<std::pair<unsigned int, unsigned int>> &pixels,
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
        unsigned int r = pixel.second;
        unsigned int c = pixel.first;
        float depth = depth_map.depth_at(r, c);
        // backproject using camera settings
        float x, y, z;
        backproject(camera, c, r, depth, &x, &y, &z);
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
    vector<vector<pair<unsigned int, unsigned int>>> valid_pixels;
    int cam_index = 0;
    for (const auto &depth_map : depth_maps) {
        vector<pair<unsigned int, unsigned int>> pixels = depth_map_to_pixels(depth_map);
        valid_pixels.push_back(pixels);
        cpd::Matrix pc = pixels_to_pointcloud(cameras.at(cam_index), pixels, depth_map);
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
    multimap<unsigned int, PixelInFrame> group_to_members;
    map<PixelInFrame, unsigned int> pixel_to_group;
    unsigned int group_idx = 0;
    for (unsigned int src_frame_idx = 0; src_frame_idx < depth_maps_as_pointclouds.size() - 1; ++src_frame_idx) {
        cout << "Mapping " << src_frame_idx << endl;

        unsigned int tgt_frame_idx = src_frame_idx + 1;
        const auto &fixed = depth_maps_as_pointclouds.at(tgt_frame_idx);
        const auto &moving = depth_maps_as_pointclouds.at(src_frame_idx);

        cpd::Nonrigid nonrigid;
        nonrigid.correspondence(true);
        nonrigid.normalize(false);
        cpd::NonrigidResult result = nonrigid.run(fixed, moving);

        for (unsigned int src_point_idx = 0; src_point_idx < result.correspondence.size(); ++src_point_idx) {
            unsigned int tgt_point_idx = result.correspondence(src_point_idx);
            pair<unsigned int, unsigned int> src_pixel = valid_pixels.at(src_frame_idx).at(src_point_idx);
            pair<unsigned int, unsigned int> tgt_pixel = valid_pixels.at(tgt_frame_idx).at(tgt_point_idx);
            PixelInFrame src_pif{src_pixel.first, src_pixel.second, src_frame_idx};
            PixelInFrame tgt_pif{tgt_pixel.first, tgt_pixel.second, tgt_frame_idx};

            auto it = pixel_to_group.find(src_pif);
            int src_group = (it == pixel_to_group.end()) ? -1 : it->second;
            it = pixel_to_group.find(tgt_pif);
            int tgt_group = (it == pixel_to_group.end()) ? -1 : it->second;

            if ( /* src group but not target group*/ src_group >= 0 && tgt_group == -1) {
                group_to_members.insert(make_pair(src_group, tgt_pif));
            } else if ( /* target group but not source group */ tgt_group >= 0 && src_group == -1) {
                group_to_members.insert(make_pair(tgt_group, src_pif));
            } else if ( /* neither group */ src_group == -1 && tgt_group == -1) {
                group_to_members.insert(make_pair(group_idx, src_pif));
                group_to_members.insert(make_pair(group_idx, tgt_pif));
                pixel_to_group.emplace(src_pif, group_idx);
                pixel_to_group.emplace(tgt_pif, group_idx);
                group_idx++;
            } else if ( /* different groups */ src_group != tgt_group) {
                // Merge the two groups
            } /* implicit else if same group do nothing */
        }
    }
    // Enumerate groups
    for (auto it = group_to_members.begin(); it != group_to_members.end();) {
        unsigned int group = it->first;
        cout << "Group : " << group << endl;
        // Advance to next non-duplicate entry.
        int count = 0;
        vector<PixelInFrame> this_group;
        do {
            ++it;
            ++count;
            cout << "[" << it->second.frame << ", " << it->second.x << ", " << it->second.y << "] ";
            this_group.push_back(it->second);
        } while (it != group_to_members.end() && group == it->first);
        correspondences.push_back(this_group);
        cout << " (" << count << " members)" << endl;
    }
    cout << group_to_members.size() << " correspondence groups found" << endl;

    // Now all PiFs are in map
    return correspondences;
}
