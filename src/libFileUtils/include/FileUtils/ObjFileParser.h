#include "Geom/geom.h"

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
	std::pair<std::vector<PointNormal::Ptr>, std::multimap<size_t, size_t>>  parse_file( const std::string& file_name, bool with_adjacency = false, bool face_wise = false );

	/**
	 * Parse an OBJ file and return only points and faces
	 * @param file_name The name of the file.
	 * @return A pair of vectors containing the points and face vertex indices.
	 */
	std::pair<std::vector<Eigen::Vector3f>, std::vector<std::vector<std::size_t>>> parse_file_raw( const std::string& file_name );

	/**
	 * Parse an OBJ file and return vertices, faces and face normals.
	 * Vertices are the 'v' elements. Face normals are computed from 'vn'
	 * @param file_name The name of the file.
	 * @return A pair of vectors containing the points and face vertex indices.
	 */
	std::pair<std::vector<Eigen::Vector3f>, std::vector<std::pair<std::vector<std::size_t>, Eigen::Vector3f>>>
 parse_file_raw_with_normals( const std::string& file_name );
};
}
