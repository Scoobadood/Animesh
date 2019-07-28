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
    /**
     * Convenience class for bundling row and column
     */
    struct PixelCoord {
        const unsigned int row;
        const unsigned int col;
        PixelCoord( const unsigned int row, const unsigned int col ) : row{row}, col{col} {
        }
        inline bool operator <(const PixelCoord& other) const{
            return ( row < other.row || (row == other.row && col < other.col));
        }
        inline bool operator ==(const PixelCoord& other) const{
            return (row == other.row && col == other.col);
        }
    };

    /**
     * Build a DepthMapPyramid based on the given map.
     * @param depth_map
     */
    explicit DepthMapPyramid(const DepthMap &depth_map);

    /**
     * Set the number of levels. If the current number of levels is less than this,
     * new levels will be generated.  If the current number of levels is greater than requested, those
     * levels will be deleted.
     * @param num_levels Desired number of levels, must be > 1.
     */
    void set_num_levels(unsigned int num_levels);

    /**
     * @return The number of levels in the pyramid.
     */
    unsigned int num_levels() const { return depth_maps.size();}

    /**
     * Get the coordinates of pixels to which the given pixel maps
     * @param level The level of the source pixel. Must be > 0.
     * @param row The row of the source pixel.
     * @param col The col of the source pixel.
     * @return A vector of pixel coords in the next layer down.
     */
    std::vector<PixelCoord> mapped_pixels(unsigned int level, unsigned int row, unsigned int col) const;


    /**
     * @param level Requested DepthMap level.
     * @return A reference to the request level.
     */
    const DepthMap& level(unsigned int level) const {
        if( level >= depth_maps.size()) {
            throw std::runtime_error( "Level out of range ");
        }
        return depth_maps[level];
    }

private:
    std::vector<DepthMap> depth_maps;

    DepthMap down_sample(const DepthMap &source_map, std::multimap<PixelCoord, PixelCoord>& mapping);

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
    std::vector<std::multimap<PixelCoord, PixelCoord>> mappings;
};

#endif //ANIMESH_DEPTHMAPPYRAMID_H
