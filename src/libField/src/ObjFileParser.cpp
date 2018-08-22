#include <Field/ObjFileParser.h>
#include <FileUtils/FileUtils.h>
#include <iostream>

const float EPSILON = 1e-4;

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
	using namespace Eigen;

	vector<Vector3f>		defined_vertices;
	vector<Vector3f> 		defined_normals;
	vector<int>  			face_vertex_idx;
	vector<int>  			face_normal_idx;
	vector<vector<size_t>>	adjacency;

	process_file_by_lines( file_name, [&](const string& line){
		if (line[0] == 'v' ) {
			// Normals
			if( line[1] == 'n') {
				istringstream iss (line);
				string ss;
				float x,y,z;
				iss >> ss >> x >> y >> z;
				Vector3f vn{x, y, z};
				cout << "vn " << x << " " << y << " " << z << endl;
				assert( abs(vn.norm() - 1.0f) < EPSILON );
				defined_normals.push_back(vn);
			} 
			// Vertices
			else {
				istringstream iss (line);
				string ss;
				float x,y,z;
				iss >> ss >> x >> y >> z;
				Vector3f v{x, y, z};
				cout << "v  " << x << " " << y << " " << z << endl;
				defined_vertices.push_back(v);
			}
		} else if( line[0] == 'f' ) {
			vector<string> tokens = split(line, ' ');
			vector<size_t> verts;
			int idx = 1;
			while( idx < tokens.size()) {
				// Form is int/int/int
				vector<string> terms = split( tokens[idx], '/');
				int v_idx = stoi(terms[0]) - 1;
				int vn_idx = stoi(terms[2]) - 1;
				face_vertex_idx.push_back(v_idx);
				face_normal_idx.push_back(vn_idx);
				verts.push_back(v_idx);
				idx++;
			}
			adjacency.push_back(verts);
		}
	});

	// Compute vertex normals from face normals
	size_t num_vertices = defined_vertices.size();
	Vector3f *computed_normals = new Vector3f[num_vertices];
	for( size_t i=0; i < num_vertices; ++i ) {
		computed_normals[i] = Vector3f::Zero();
	}
	for( size_t i = 0; i < face_vertex_idx.size(); ++i ) {
		size_t vertex_idx = face_vertex_idx[i];
		assert( vertex_idx < num_vertices);

		size_t normal_idx = face_normal_idx[i];
		assert( normal_idx < defined_normals.size());

		computed_normals[vertex_idx] += defined_normals[normal_idx];
	}

	// Stash verts and norms
	for( size_t i = 0; i < num_vertices; ++i ) {
		m_vertices.push_back(defined_vertices[i]);

		Vector3f vn = computed_normals[i];
		cout << "Normalising this : " << vn[0] << " " << vn[1] << " " << vn[2] << endl;
		vn.normalize();
		cout << "Gives this : " << vn[0] << " " << vn[1] << " " << vn[2] << "(norm is :" << vn.norm() << ")" << endl;
		assert( abs(vn.norm() - 1.0f) < EPSILON);
		m_normals.push_back(vn);

		vector<size_t> adj;
		m_adjacency.push_back(adj);
		std::cout << i << ":" << vn << std::endl;
	}
	delete [] computed_normals;

	// Finally compute adjacency
	for( auto adj : adjacency ) {
		size_t n = adj.size();
		for( size_t idx=0; idx < n; ++idx ) {
			size_t first = adj[idx];
			assert( first < num_vertices);
			size_t second = adj[(idx + 1) % n];
			assert( second < num_vertices);
			m_adjacency[first].push_back(second);
			m_adjacency[second].push_back(first);
		}
	}
}
