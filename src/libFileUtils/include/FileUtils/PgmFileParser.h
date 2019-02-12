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
};

Frame load_frame_from_file( const std::string& file_name );