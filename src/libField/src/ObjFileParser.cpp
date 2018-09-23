
#include <Field/ObjFileParser.h>
#include <Field/PointNormal.h>
#include <FileUtils/FileUtils.h>
#include <iostream>
#include <map>


using animesh::ObjFileParser;
using animesh::PointNormal;

const float EPSILON = 1e-4;

template <typename Out>
void split(const std::string &s, char delim, Out result) {
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        *(result++) = item;
    }
}

std::vector<std::string>
split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    split(s, delim, std::back_inserter(elems));
    return elems;
}

void
handle_vertex_line( const std::string& line, std::vector<Eigen::Vector3f>& vertices ) {
	using namespace std;
	using namespace Eigen;

	istringstream iss (line);
	string ss;
	float x,y,z;
	iss >> ss >> x >> y >> z;
	Vector3f v{x, y, z};
	vertices.push_back(v);
}

void
handle_normal_line( const std::string& line, std::vector<Eigen::Vector3f>& normals ) {
	using namespace std;
	using namespace Eigen;

	istringstream iss (line);
	string ss;
	float x,y,z;
	iss >> ss >> x >> y >> z;
	Vector3f vn{x, y, z};
	assert( abs(vn.norm() - 1.0f) < EPSILON );
	normals.push_back(vn);
}

/**
 * Handle lines of the form f v//vn v//vn v//vn
 * Where each is a face and v are the indices of vertices and vn of normals
 */
void
handle_face_line_with_adjacency( const std::string& line, std::vector<size_t>& face_vertex_idx, std::vector<size_t>& face_normal_idx, std::vector<std::vector<size_t>>& face_vertices) {
	using namespace std;
	using namespace Eigen;

	vector<string> tokens = split(line, ' ');
	vector<size_t> verts;
	size_t idx = 1;
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
	face_vertices.push_back(verts);
}

void
handle_face_line_without_face_vertices( const std::string& line, std::vector<size_t>& face_vertex_idx, std::vector<size_t>& face_normal_idx) {
	using namespace std;
	using namespace Eigen;

	vector<string> tokens = split(line, ' ');
	size_t idx = 1;
	while( idx < tokens.size()) {
		// Form is int/int/int
		vector<string> terms = split( tokens[idx], '/');
		int v_idx = stoi(terms[0]) - 1;
		int vn_idx = stoi(terms[2]) - 1;
		face_vertex_idx.push_back(v_idx);
		face_normal_idx.push_back(vn_idx);
		idx++;
	}
}


/**
 * Given a set of all face indices and face normal indices
 * Compute the mean vertex normals for each given vertex by
 * averaging the normal for that vertex over every face it appears in.
 *
 */
void
compute_vertex_normals( size_t num_vertices,
						const std::vector<size_t>& face_vertex_indices,
						const std::vector<size_t>& face_normal_indices,
						const std::vector<Eigen::Vector3f>& given_normals,
						std::vector<Eigen::Vector3f>& vertex_normals ) {
	using namespace Eigen;

	assert( num_vertices > 0 );
	assert( face_vertex_indices.size() == face_normal_indices.size());

	for( size_t i=0; i < num_vertices; ++i ) {
		vertex_normals.push_back( Vector3f::Zero() );
	}

	for( size_t i = 0; i < face_vertex_indices.size(); ++i ) {
		size_t vertex_idx = face_vertex_indices[i];
		size_t normal_idx = face_normal_indices[i];

		vertex_normals[vertex_idx] += given_normals[normal_idx];
	}
}


/**
 * Parse an OBJ file and return all PointNormals and face_vertices.
 * @param file_name The name of the file.
 * @return A vector containing the points and a vector of adjacent point indices.
 */
std::pair<std::vector<PointNormal::Ptr>, std::multimap<size_t, size_t>>
ObjFileParser::parse_file_with_adjacency( const std::string& file_name, bool face_wise ) {
	using namespace std;
	using namespace Eigen;

	vector<Vector3f>		given_vertices;
	vector<Vector3f> 		given_normals;
	vector<size_t>  		face_vertex_indices;
	vector<size_t>  		face_normal_indices;
	vector<vector<size_t>>	face_vertices;

	process_file_by_lines( file_name, [&](const string& line){
		if (line[0] == 'v' ) {
			// Normals
			if( line[1] == 'n') {
				handle_normal_line( line, given_normals);
			}
			// Vertices
			else {
				handle_vertex_line( line, given_vertices );
			}
		} else if( line[0] == 'f' ) {
			handle_face_line_with_adjacency( line, face_vertex_indices, face_normal_indices, face_vertices);
		}
	});

	// Compute points and normals
	size_t num_vertices = given_vertices.size();
	vector<Eigen::Vector3f> vertex_normals;
	compute_vertex_normals( num_vertices, face_vertex_indices, face_normal_indices, given_normals, vertex_normals );

	// Stash verts and norms
	vector<PointNormal::Ptr> point_normals;
	for( size_t i = 0; i < num_vertices; ++i ) {
		PointNormal::Ptr pnp{new PointNormal(given_vertices[i], vertex_normals[i].normalized())};
		point_normals.push_back(pnp);
	}

	// Finally compute face_vertices
	multimap<size_t, size_t> adjacency;
	for( auto adj : face_vertices ) {
		size_t n = adj.size();
		for( size_t idx=0; idx < n; ++idx ) {
			size_t first = adj[idx];
			assert( first < num_vertices);
			size_t second = adj[(idx + 1) % n];
			assert( second < num_vertices);
			adjacency.insert( make_pair( first, second));
		}
	}
	return make_pair(point_normals, adjacency);
}

/**
 * Parse an OBJ file and return only PointNormals.
 * @param file_name The name of the file.
 * @return A vector containing the points and normals.
 */
std::vector<PointNormal::Ptr>
ObjFileParser::parse_file( const std::string& file_name, bool face_wise ) {
	using namespace std;
	using namespace Eigen;

	vector<Vector3f>		given_vertices;
	vector<Vector3f> 		given_normals;
	vector<size_t>  		face_vertex_indices;
	vector<size_t>  		face_normal_indices;

	process_file_by_lines( file_name, [&](const string& line){
		if (line[0] == 'v' ) {
			// Normals
			if( line[1] == 'n') {
				handle_normal_line( line, given_normals);
			}
			// Vertices
			else {
				handle_vertex_line( line, given_vertices );
			}
		} else if( line[0] == 'f' ) {
			handle_face_line_without_face_vertices( line, face_vertex_indices, face_normal_indices);
		}
	});

	// Compute points and normals
	size_t num_vertices = given_vertices.size();
	vector<Eigen::Vector3f> vertex_normals;
	compute_vertex_normals( num_vertices, face_vertex_indices, face_normal_indices, given_normals, vertex_normals );

	// Stash verts and norms
	vector<PointNormal::Ptr> results;
	for( size_t i = 0; i < num_vertices; ++i ) {
		PointNormal::Ptr pnp{new PointNormal(given_vertices[i], vertex_normals[i].normalized())};
		results.push_back( pnp );
	}
	return results;
}
