#include <vector>
#include <string>
#include <Eigen/Core>

class ObjFileParser {
public:

	ObjFileParser(std::string file_name);

	std::vector<Eigen::Vector3f>		m_vertices;
	std::vector<Eigen::Vector3f>		m_normals;
	std::vector<std::vector<size_t>>	m_adjacency;
};
