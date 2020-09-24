#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <cassert>
#include <Camera/Camera.h>
#include <DepthMap/Normals.h>

class DepthMap {
public:
	typedef enum {
		UP_LEFT = 1,
		UP = 2,
		UP_RIGHT = 4,
		RIGHT = 8,
		DOWN_RIGHT = 16,
		DOWN = 32,
		DOWN_LEFT = 64,
		LEFT = 128,
		ALL = 255,
		FOUR = UP | LEFT | RIGHT | DOWN
	} tDirection;

    /**
	 * Load from file.
	 */
	DepthMap(const std::string& filename);

	/**
	 * Construct from an array of floats and dimensions
	 * @param rows The number of rows provided.
	 * @param cols The number of columns provided.
	 * @param depth_data a rows*cols, row major set of depths.
	 */
	 DepthMap(unsigned int width, unsigned int height, float * depth_data);

	 /** @return the height of the depth map. */
	inline unsigned int height() const { return this->m_height;}

	/** @return the width of the depth map. */
	inline unsigned int width() const { return m_width;}

	/**
	 * @param x
	 * @param y
	 *  @return the depth at the giovemn coordinate.
	 */
	float depth_at(unsigned int x, unsigned int y) const {
        assert(x < m_width && x >= 0 );
		assert(y < m_height && y >= 0 );
		return m_depth_data[index(x, y)];
	}

    /**
     * Subsample a depth map and return a map that is half the size (rounded down) in each dimension.
     * Entries in the resulting map are computed from the mean of entries in this map.
     */
    DepthMap resample() const;

	void cull_unreliable_depths(float ts, float tl);
    const std::vector<std::vector<NormalWithType>> & get_normals() const;
	static inline bool flag_is_set( unsigned int flags, DepthMap::tDirection flag ){
        return ((flags & flag) == flag);
    }
	bool is_normal_defined(unsigned int x, unsigned int y) const;

    NormalWithType normal_at(unsigned int x, unsigned int y) const;
    void compute_normals(const Camera& camera, tNormalMethod method);

//	inline bool is_edge(unsigned int row, unsigned int col) const {
//		return (row == 0 || row == rows() - 1 || col == 0 || col == cols() - 1);
//	}

/**
 * Given a row and column, get the depths of adjacent pixels in the depth map.
 * If eightConnected is true, consider diagonal neighbours, otherwise only left irght up and down.
 * Populates neighbour_depths with depth values (or 0.0) and returns a flag in indicating
 * which values are valid.
 * Values in neighbour depths are ordered:
 * UP, DOWN, LEFT, RIGHT, UP_LEFT, UP_RIGHT, DOWN_LEFT, DOWN_RIGHT
 */
	unsigned int get_neighbour_depths(unsigned int x, unsigned int y, float neighbour_depths[], bool eightConnected = false) const;

protected:

    /**
     * Given an (X,Y) coordinate in a depth map, return a set of flags indicating which
     * directions from this coordinate are valid (in the sense that they fall inside the
     * depth map.
     * @param x X coordinate.
     * @param y Y coordinate.
     * @param eightConnected If true, consider diagonally adjacent pixels, otherwise just horizontal and vertical.
     * @return
     */
    unsigned int get_valid_directions(unsigned int x, unsigned int y, bool eightConnected) const;


private:
	float *m_depth_data;
	unsigned int m_width;
	unsigned int m_height;

	// row, col
	std::vector<std::vector<NormalWithType>> normals;

	/**
	 * Compute the index into depth_map data for a given (x,y) coordinate.
	 * @param y The y coordinate
	 * @param x Tghe x coorinate.
	 * @return The offset.
	 */
	inline unsigned int index(unsigned int x, unsigned int y) const {
        return y * m_width + x;
    }

};
