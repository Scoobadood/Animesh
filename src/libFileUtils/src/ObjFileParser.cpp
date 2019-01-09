
#include <FileUtils/ObjFileParser.h>
#include <FileUtils/FileUtils.h>
#include <Geom/geom.h>
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
handle_face_line(
  const std::string& line,
  std::vector<size_t>& face_vertex_idx,
  std::vector<size_t>& face_normal_idx,
  std::vector<std::vector<std::pair<std::size_t, std::size_t>>>& faces) {
	using namespace std;
	using namespace Eigen;

	vector<string> tokens = split(line, ' ');
	vector<pair<size_t, size_t>> verts;
	size_t idx = 1;
	while( idx < tokens.size()) {
		// Form is int/int/int
		vector<string> terms = split( tokens[idx], '/');
		int v_idx = stoi(terms[0]) - 1;
		int vn_idx = stoi(terms[2]) - 1;
		face_vertex_idx.push_back(v_idx);
		face_normal_idx.push_back(vn_idx);
		verts.push_back(make_pair(v_idx, vn_idx));
		idx++;
	}
	faces.push_back(verts);
}

void
read_data(const std::string&                      file_name,
          std::vector<Eigen::Vector3f>&           given_vertices,
          std::vector<Eigen::Vector3f>&           given_normals,
          std::vector<std::size_t>&               face_vertex_indices,
          std::vector<std::size_t>&               face_normal_indices,
          std::vector<std::vector<std::pair<std::size_t, std::size_t>>>& faces) {
  using namespace std;
	using namespace Eigen;


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
      handle_face_line( line, face_vertex_indices, face_normal_indices, faces);
    }
  });
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

std::vector<PointNormal::Ptr>
compute_point_normals_from_vertices(
  const std::vector<Eigen::Vector3f>&   given_vertices,
  const std::vector<Eigen::Vector3f>&   given_normals,
  const std::vector<std::size_t>&       face_vertex_indices,
  const std::vector<std::size_t>&       face_normal_indices ) {

    using namespace std;

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

  return point_normals;
}

// Finally compute face_vertices
std::multimap<std::size_t, std::size_t>
compute_adjacency_from_vertices( const std::vector<std::vector<std::pair<std::size_t, std::size_t>>>& faces ) {
  using namespace std;

  multimap<size_t, size_t> adjacency;

  for( auto face : faces ) {
    size_t n = face.size();
    for( size_t idx=0; idx < n; ++idx ) {
      size_t first = face[idx].first;
      size_t second = face[(idx + 1) % n].first;
      adjacency.insert( make_pair( first, second));
    }
  }
  return adjacency;
}


std::vector<PointNormal::Ptr>
compute_point_normals_from_faces(
  const std::vector<Eigen::Vector3f>&           given_vertices,
  const std::vector<Eigen::Vector3f>&           given_normals,
  const std::vector<std::vector<std::pair<std::size_t, std::size_t>>>& faces) {

    using namespace std;
    using namespace Eigen;

    vector<PointNormal::Ptr> point_normals;

    // For each face, compute centre and normal and edge maps
    for( size_t face_idx = 0; face_idx < faces.size(); ++face_idx ) {
      auto face_vertex_normals = faces[face_idx];
      Vector3f face_centre = Vector3f::Zero();
      Vector3f face_normal = Vector3f::Zero();
      for( size_t face_vertex_idx = 0; face_vertex_idx < face_vertex_normals.size(); ++face_vertex_idx) {
        auto face_vertex_normal = face_vertex_normals[face_vertex_idx];
        face_centre = face_centre + given_vertices[face_vertex_normal.first];
        face_normal = face_normal + given_normals[face_vertex_normal.second];
      }
      // Make the normal and centre
      PointNormal::Ptr pnp{new PointNormal( face_centre / face_vertex_normals.size(), face_normal.normalized()  )};
      point_normals.push_back(pnp);
    }

    return point_normals;
  }

/**
 * Determine adjacency for nodes where each node is a face based on shared edges in then
 * graph.
 * A face is adjacent to another face if they share an edge.
 * Actually sharing more than one vertex is sufficient as this defines an edge
 * We have a vector of vectors of vertex indices
 * We want a multimap of
 */
std::multimap<std::size_t, std::size_t>
compute_adjacency_from_faces( const std::vector<std::vector<std::pair<std::size_t, std::size_t>>>& faces ) {
  using namespace std;

  // Construct edges->faces map
  map<pair<size_t, size_t>, vector<size_t>> edge_to_faces;
  for( size_t face_idx = 0; face_idx < faces.size(); ++face_idx ) {
    for( size_t face_vertex_idx = 0; face_vertex_idx < faces[face_idx].size(); ++face_vertex_idx ) {
        size_t from_idx = faces[face_idx][face_vertex_idx].first;
        size_t to_idx = faces[face_idx][(face_vertex_idx + 1) % faces[face_idx].size()].first;
        pair<size_t, size_t> edge = from_idx < to_idx ? make_pair(from_idx, to_idx) : make_pair( to_idx, from_idx );
        edge_to_faces[edge].push_back( face_idx);
    }
  }

  // For each edge, all faces that share it are adjacent.
  multimap<size_t, size_t> adjacency;
  for( auto edge_face : edge_to_faces) {
    vector<size_t> adjacent_faces = edge_face.second;
    // Shared edge or boundary edge
    assert( adjacent_faces.size() == 2 || adjacent_faces.size() == 1);
    if( adjacent_faces.size() == 2) {
      adjacency.insert(make_pair(adjacent_faces[0], adjacent_faces[1]));
    }
  }
  return adjacency;
}

/**
 * Parse an OBJ file and return all PointNormals and face_vertices.
 * @param file_name The name of the file.
 * @return A vector containing the points and a vector of adjacent point indices.
 */
std::pair<std::vector<PointNormal::Ptr>, std::multimap<size_t, size_t>>
ObjFileParser::parse_file( const std::string& file_name, bool with_adjacency, bool face_wise ) {
	using namespace std;
	using namespace Eigen;

	vector<Vector3f>		given_vertices;
	vector<Vector3f> 		given_normals;
	vector<size_t>  		face_vertex_indices;
	vector<size_t>  		face_normal_indices;
	vector<vector<pair<size_t,size_t>>>	faces;

  read_data( file_name, given_vertices, given_normals, face_vertex_indices, face_normal_indices, faces );

  vector<PointNormal::Ptr> point_normals;
  multimap<size_t, size_t> adjacency;

  if( face_wise) {
    point_normals = compute_point_normals_from_faces(given_vertices, given_normals, faces);
    if( with_adjacency) {
      adjacency = compute_adjacency_from_faces(faces);
    }
  } else {
    point_normals = compute_point_normals_from_vertices( given_vertices, given_normals, face_vertex_indices, face_normal_indices );
    if( with_adjacency) {
      adjacency = compute_adjacency_from_vertices(faces);
    }
  }

	return make_pair(point_normals, adjacency);
}

/**
 * Parse an OBJ file and return only points and faces
 * @param file_name The name of the file.
 * @return A pair of vectros containing the points and face vertex indices.
 */
std::pair<std::vector<Eigen::Vector3f>, std::vector<std::vector<std::size_t>>>
ObjFileParser::parse_file_raw( const std::string& file_name ) {
  using namespace std;
  using namespace Eigen;

  vector<Vector3f>		given_vertices;
	vector<Vector3f> 		given_normals;
	vector<size_t>  		face_vertex_indices;
	vector<size_t>  		face_normal_indices;
	vector<vector<pair<size_t,size_t>>>	faces;
  read_data( file_name, given_vertices, given_normals, face_vertex_indices, face_normal_indices, faces );
  vector<vector<size_t>> face_points;
  for( auto face_data : faces ) {
    vector<size_t> face_point_data;
    for( auto face_point_normal : face_data ) {
      face_point_data.push_back( face_point_normal.first);
    }
    face_points.push_back( face_point_data );
  }
  return make_pair( given_vertices, face_points );
}
