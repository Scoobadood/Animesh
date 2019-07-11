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
    m /= c;
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
DepthMapPyramid::down_sample(const DepthMap& source_map) {
    using namespace std;

    int new_rows = source_map.rows() / 2;
    int new_cols= source_map.cols() / 2;
    auto new_data = new float[new_rows * new_cols];

    for( int r = 0; r < new_rows; ++r) {
        for( int c = 0; c < new_cols; ++c ) {
            unsigned int source_row = r * 2;
            unsigned int source_col = c * 2;
            float values[4];
            values[0] = source_map.depth_at(source_row,                                 source_col);
            values[1] = source_map.depth_at(min(source_row + 1, source_map.rows() - 1), source_col);
            values[2] = source_map.depth_at(source_row,                                 min(source_col + 1, source_map.cols() - 1));
            values[3] = source_map.depth_at(min(source_row + 1, source_map.rows() - 1), min(source_col + 1, source_map.cols() - 1));
            new_data[r * new_rows ] = merge(values);
        }
    }
    return DepthMap{ new_rows, new_cols, new_data };
}

/**
 * Set the number of levels. Recompute new levels as necessary.
 * @param num_levels The number of levels, minimum of 1.
 */
void 
DepthMapPyramid::set_num_levels(int num_levels) {
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