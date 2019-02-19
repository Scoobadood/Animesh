/**
 * Loads Frames from data
 */
#pragma once
#include <vector>
#include <string>
#include <FileUtils/FileUtils.h>

struct Frame {
	std::size_t 		width;
	std::size_t 		height;
	std::vector<int>	data;

	bool isValidAt( std::size_t x, std::size_t y ) {
		if( x > width || y > height ) return false;
		if( data[y * width + height] == 0 ) return false;
		return true;
	}
};

Frame load_frame_from_file( const std::string& file_name );