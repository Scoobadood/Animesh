#include <vector>
#include <string>

class ObjFileParser {
public:
	struct vec3 {
		float x, y, z;
		vec3( ) {
			x = 0.0f;
			y = 0.0f;
			z = 0.0f;
		}
		vec3( float x, float y, float z ) {
			this->x = x;
			this->y = y;
			this->z = z;
		}
	};

size_t num_vertices() const;
vec3 vertex_at(size_t i) const;
vec3 normal_at(size_t i) const;
std::vector<size_t> vertices_adjacent_to(size_t i) const;

private:
	std::vector<vec3>				m_vertices;
	std::vector<vec3>				m_normals;
	std::vector<std::vector<int>>	m_adjacency;
}
