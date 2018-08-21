#include <ObjFileParser.h>
#include <FileUtils/FileUtils.h>

template <typename Out>
void split(const std::string &s, char delim, Out result) {
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        *(result++) = item;
    }
}

std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    split(s, delim, std::back_inserter(elems));
    return elems;
}


ObjFileParser::ObjFileParser(std::string file_name) {
	using namespace std;

	vector<vec3> 			defined_vertices;
	vector<vec3> 			defined_normals;
	vector<int>  			face_vertex_idx;
	vector<int>  			face_normal_idx;
	vector<vector<size_t>>	adjacency;

	process_file_by_lines( file_name, [&](const string& line){
		if (line[0] == 'v' ) {
			if( line[1] == 'n') {
				istringstream iss (line);
				string ss;
				float x,y,z;
				iss >> ss >> x >> y >> z;
				vec3 vn{x, y, z};
				defined_normals.push_back(vn);
			} else {
				istringstream iss (line);
				string ss;
				float x,y,z;
				iss >> ss >> x >> y >> z;
				vec3 v{x, y, z};
				defined_vertices.push_back(v);
			}
		} else if( line[0] == 'f' ) {
			vector<string> tokens = split(line, ' ');
			vector<size_t> verts;
			int idx = 1;
			while( idx < tokens.size()) {
				// Form is int/int/int
				vector<string> terms = split( tokens[idx], '/');
				int v_idx = stoi(terms[0]);
				int vn_idx = stoi(terms[2]);
				face_vertex_idx.push_back(v_idx-1);
				face_normal_idx.push_back(vn_idx-1);
				verts.push_back(v_idx-1);
				idx++;
			}
			adjacency.push_back(verts);
		}
	});

	size_t num_vertices = defined_vertices.size();
	vec3 *computed_normals = new vec3[num_vertices];

	for( size_t i = 0; i < face_vertex_idx.size(); ++i ) {
		int vertex_idx = face_vertex_idx[i];
		assert( vertex_idx >= 0 && vertex_idx < num_vertices);

		int normal_idx = face_normal_idx[i];
		assert( normal_idx >= 0 && normal_idx < defined_normals.size());

		vec3 current_norm = computed_normals[vertex_idx];
		vec3 addin_norm = defined_normals[normal_idx];
		current_norm.x += addin_norm.x;
		current_norm.y += addin_norm.y;
		current_norm.z += addin_norm.z;
		computed_normals[vertex_idx] = current_norm;
	}

	for( size_t i = 0; i < num_vertices; ++i ) {
		vec3 v = defined_vertices[i];
		m_vertices.push_back(v);

		vec3 vn = computed_normals[i];
		float len = std::sqrt(vn.x*vn.x + vn.y*vn.y + vn.z*vn.z);
		assert( len > 1e-6 );
		vn.x /= len;
		vn.y /= len;
		vn.z /= len;
		m_normals.push_back(vn);

		vector<size_t> adj;
		m_adjacency.push_back(adj)
	}
	delete [] computed_normals;

	// Finally compute adjacency
	for( auto adj : adjacency ) {
		size_t n = adj.size();
		for( size_t idx=0; i<n; ++i ) {
			size_t first = adj[idx];
			size_t second = adj[(idx + 1) % n];
			m_adjacency[first].push_back(second);
			m_adjacency[second].push_back(first);
		}
	}
}

size_t 
ObjFileParser::num_vertices() const {
	return m_vertices.size();
}

vec3
ObjFileParser::vertex_at(size_t i) const {
	return m_vertices[i];
}

vec3
ObjFileParser::normal_at(size_t i) const {
	return m_normals[i];
}

std::vector<size_t> 
ObjFileParser::vertices_adjacent_to(size_t i) const {
	return m_adjacency[i];
}
