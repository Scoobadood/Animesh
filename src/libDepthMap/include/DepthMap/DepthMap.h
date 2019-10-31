#pragma once

#include <string>
#include <vector>
#include <cassert>

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

    typedef enum {
        NONE,
        DERIVED,
        NATURAL
    } tNormal;

    struct NormalWithType {
        tNormal type;
        float x;
        float y;
        float z;
        NormalWithType(tNormal t, float xx, float yy, float zz) : type{t}, x{xx}, y{yy}, z{zz} {};
    };

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
	 DepthMap(unsigned int rows, unsigned int cols, float * depth_data);

	inline unsigned int rows() const { return height;}
	inline unsigned int cols() const { return width;}
	float depth_at(unsigned int row, unsigned int col) const {
		assert( row < height && row >= 0 );
		assert( col < width && col >= 0 );
		return depth_data[index(row, col)];
	}
	void cull_unreliable_depths(float ts, float tl);
	const std::vector<std::vector<std::vector<float>>>& get_normals() const;
	int get_valid_directions(unsigned int row, unsigned int col, bool eightConnected) const;
	static bool flag_is_set( int flags, DepthMap::tDirection flag );
	bool is_normal_defined(unsigned int row, unsigned int col) const;

	/**
	 * Subsample a depth map and return a map that is half the size (rounded down) in each dimension.
	 * Entries in the resulting map are computed from the mean of entries in this map.
	 */
	DepthMap resample() const;
    NormalWithType normal_at(unsigned int x, unsigned int y) const;


private:
	float *depth_data;
	unsigned int width;
	unsigned int height;

	// row, col
	std::vector<std::vector<std::vector<float>>> normals;
	// row, col
	std::vector<std::vector<tNormal>> normal_types;

	void compute_normals();
	void compute_natural_normals();
	void compute_derived_normals();

	inline unsigned int index(unsigned int row, unsigned int col) const {
		return row * width + col;
	}
	inline bool is_edge(unsigned int row, unsigned int col) const {
		return (row == 0 || row == rows() - 1 || col == 0 || col == cols() - 1);
	}
	int get_neighbour_depths(unsigned int row, unsigned int col, float neighbour_depths[], bool eightConnected = false) const;
};
