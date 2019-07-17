//
// Created by Dave Durbin on 2019-07-07.
//

#include "DepthMap/DepthMapPyramid.h"

DepthMapPyramid::DepthMapPyramid(const DepthMap& depth_map) {
    depth_maps.push_back(depth_map);
}

/**
 * Merge four values into a single depth value to propagate up.
 * Compute mean of non-zero values.
 */
float
merge(const float * source_values) {
    float m = 0.0f;
    float c = 0;
    for( int i=0; i<4; ++i ) {
        if( source_values[i] > 0.0f ) {
            m += source_values[i];
            c ++;
        }
    }
    m = (c != 0 ) ? m / c  : 0;
    return m;
}

/**
 * Downsample a Depth Map.
 * Generate a new depth map with half the edge length of the source one.
 * New depths are computed as the mean of the
 * @param source_map
 * @return
 */
DepthMap
DepthMapPyramid::down_sample(const DepthMap &source_map, std::multimap<std::pair<int,int>, std::pair<int,int>>& mapping) {
    using namespace std;

    int new_rows = source_map.rows() / 2;
    int new_cols= source_map.cols() / 2;
    auto new_data = new float[new_rows * new_cols];

    for( int r = 0; r < new_rows; ++r) {
        for( int c = 0; c < new_cols; ++c ) {
            unsigned int source_row = r * 2;
            unsigned int source_col = c * 2;
            float values[4];
            pair<int,int> mapped_pixels[4];

            mapped_pixels[0] = make_pair(source_row,                                 source_col);
            mapped_pixels[1] = make_pair(min(source_row + 1, source_map.rows() - 1), source_col);
            mapped_pixels[2] = make_pair(source_row,                                 min(source_col + 1, source_map.cols() - 1));
            mapped_pixels[3] = make_pair(min(source_row + 1, source_map.rows() - 1), min(source_col + 1, source_map.cols() - 1));

            for( int i= 0; i<4; ++i ) {
                values[i] = source_map.depth_at(mapped_pixels[i].first, mapped_pixels[i].second);
                mapping.insert(make_pair(make_pair(r, c), mapped_pixels[i]));
            }
            new_data[r * new_rows + c ] = merge(values);
        }
    }
    return DepthMap{ new_rows, new_cols, new_data };
}

/**
 * Set the number of levels. Recompute new levels as necessary.
 * @param num_levels The number of levels, minimum of 1.
 */
void 
DepthMapPyramid::set_num_levels(unsigned int num_levels) {
    using namespace std;

    if( num_levels < 1 ) {
        throw runtime_error("Can't have less than one level for a DepthMapPyramid");
    }
    if( depth_maps.size() > num_levels) {
        remove_levels(num_levels);
        return;
    }
    while( depth_maps.size() < num_levels) {
        make_new_level();
    }
}

/**
 * Delete unwanted levels and their mappings
 */
void
DepthMapPyramid::remove_levels(int from_level) {
    mappings.erase( mappings.begin() + from_level, mappings.end());
    depth_maps.erase( depth_maps.begin() + from_level, depth_maps.end());
}


void
DepthMapPyramid::make_new_level() {
    using namespace std;

    if(depth_maps.back().rows() < 2 || depth_maps.back().cols() < 2 ) {
        throw runtime_error("Too small to downsize");
    }
    multimap<pair<int,int>, pair<int,int>> mapping;
    DepthMap dm = down_sample(depth_maps.back(), mapping);
    depth_maps.push_back(dm);
    mappings.push_back(mapping);
}

