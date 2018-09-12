#include "Field/PointNormal.h"

#include <vector>
#include <string>
#include <Eigen/Core>

namespace animesh {


class ObjFileParser {
public:
	/**
	 * Parse an OBJ file and return all PointNormals and adjacency.
	 * @param file_name The name of the file.
	 * @return A vector containing the points and a vector of adjacent point indices.
	 */
	std::pair<std::vector<PointNormal::Ptr>, std::multimap<size_t, size_t>>  parse_file_with_adjacency( const std::string& file_name );

	/**
	 * Parse an OBJ file and return only PointNormals.
	 * @param file_name The name of the file.
	 * @return A vector containing the points and normals.
	 */
	std::vector<PointNormal::Ptr> parse_file( const std::string& file_name );
};


}