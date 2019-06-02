#pragma once

#include <string>
#include <vector>


class DepthMap {
public:
	/**
	 * Load from file.
	 */
	DepthMap(const std::string& filename);

	inline unsigned int rows() const { return height;}
	inline unsigned int cols() const { return width;}
	float depth_at(unsigned int row, unsigned int col) const {
		assert( row < height && row >= 0 );
		assert( col < width && col >= 0 );
		return depth_data[index(row, col)];
	}
	void cull_unreliable_depths(float ts, float tl);
	const std::vector<std::vector<std::vector<float>>>& compute_normals();

private:
	typedef enum{
		NONE,
		DERIVED,
		NATURAL
	} tNormal;
	typedef enum{
		UP = 1,
		DOWN = 2,
		LEFT = 4,
		RIGHT = 8,
		ALL = 15
	} tDirection;

	float *depth_data;
	unsigned int width;
	unsigned int height;
	std::vector<std::vector<std::vector<float>>> normals;
	std::vector<std::vector<tNormal>> normal_types;
	inline unsigned int index(unsigned int row, unsigned int col) const {
		return row * width + col;
	}
	inline bool is_edge(unsigned int row, unsigned int col) const {
		return (row == 0 || row == rows() - 1 || col == 0 || col == cols() - 1);
	}
	inline int get_neighbour_depths(unsigned int row, unsigned int col, float neighbour_depths[]) const {
		int flags = 0;
		if( row != 0 ) {
			neighbour_depths[0] = depth_at(row-1, col);
			flags |= UP;
		}
		if( row != rows() - 1 ) {
			neighbour_depths[1] = depth_at(row+1, col);
			flags |= DOWN;
		}
		if( col != 0 ) {
			neighbour_depths[2] = depth_at(row, col-1);
			flags |= LEFT;
		}
		if( col != cols() - 1 ) {
			neighbour_depths[3] = depth_at(row, col+1);
			flags |= RIGHT;
		}
		return flags;
	}
};