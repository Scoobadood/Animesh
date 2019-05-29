#pragma once

#include <string>

class DepthMap {
public:
	/**
	 * Load from file.
	 */
	DepthMap(const std::string& filename);

	inline unsigned int rows() { return height;}
	inline unsigned int cols() { return width;}
	float depth_at(unsigned int row, unsigned int col) {
		assert( row < height && row >= 0 );
		assert( col < width && col >= 0 );
		return depth_data[index(row, col)];
	}
	void cull_unreliable_depths(float ts, float tl);

private:
	float *depth_data;
	unsigned int width;
	unsigned int height;
	inline unsigned int index(unsigned int row, unsigned int col) {
		return row * width + col;
	}
};