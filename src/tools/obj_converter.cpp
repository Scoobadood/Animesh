#include <FileUtils/FileUtils.h>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <iterator>
#include <cmath>

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

size_t num_verts;

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

int main( int argc, char * argv[] ) {

	using namespace std;

	vector<vec3> defined_vertices;
	vector<vec3> defined_normals;
	vector<int>  face_vertex_idx;
	vector<int>  face_normal_idx;

	if( argc == 2 ) {
		string infile_name = argv[1];
		process_file_by_lines( infile_name, [&](const string& line){
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
				int idx = 1;
				while( idx < tokens.size()) {
					// Form is int/int/int
					vector<string> terms = split( tokens[idx], '/');
					int v_idx = stoi(terms[0]);
					int vn_idx = stoi(terms[2]);
					face_vertex_idx.push_back(v_idx-1);
					face_normal_idx.push_back(vn_idx-1);
					idx++;
				}
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
			cout << "v " << v.x << " " << v.y << " " << v.z << endl;
		}

		for( size_t i = 0; i < num_vertices; ++i ) {
			vec3 vn = computed_normals[i];

			float len = std::sqrt(vn.x*vn.x + vn.y*vn.y + vn.z*vn.z);
			assert( len > 1e-6 );
			vn.x /= len;
			vn.y /= len;
			vn.z /= len;

			cout << "vn " << vn.x << " " << vn.y << " " << vn.z << endl;
		}
		delete [] computed_normals;
	}
}