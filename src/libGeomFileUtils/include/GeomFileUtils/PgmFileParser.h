/**
 * Loads PGM files
 */
#pragma once

#include <vector>
#include <string>

struct PgmData {
	std::size_t 		width;
	std::size_t 		height;
	std::vector<int>	data;
};

/**
 * Read the provided file and return a PgmData object.
 * File should be a type P2 at this point.
 */
PgmData read_pgm( const std::string& file_name );