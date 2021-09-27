//
// Created by Dave Durbin (Old) on 26/9/21.
//

#include "GeomFileUtils/PlyFileParser.h"
#include "GeomFileUtils/io_utils.h"
#include <fstream>

namespace animesh {
const static std::vector<std::string> VALID_PROPERTY_TYPES{
    "char", "uchar", "short", "ushort",
    "int", "uint", "float", "double",
};
const static std::vector<std::string> VALID_LIST_PROPERTY_TYPES{
    "char", "uchar", "short", "ushort",
    "int", "uint",
};

bool starts_with(const std::string &line, const std::string &prefix) {
  return line.rfind(prefix, 0) == 0;
}

std::string read_line_skipping_comments(std::istream &file_stream) {
  std::string line;
  while (getline(file_stream, line)) {
    if (line.empty()) {
      continue;
    }
    if (!starts_with(line, "comment")) {
      break;
    }
  }
  return line;
}

void parse_format(const std::string &format_line, animesh::PlyFileParser::PlyHeader &header) {
  using namespace std;

  vector<string> tokens = split(format_line, ' ');

  /* Should be  one of
  format ascii 1.0
  format binary_little_endian 1.0
  format binary_big_endian 1.0
  */

  if (tokens.size() != 3) {
    throw std::runtime_error("Invalid format line \"" + format_line
                                 + "\" expected \"format <ascii|binary_little_endian|binary_big_endian> 1.0");
  }
  if (tokens[2] != "1.0") {
    throw std::runtime_error("Invalid version \"" + tokens[2] + "\", expected 1.0");
  }
  header.version = 1;
  if (tokens[1] == "ascii") {
    header.is_ascii = true;
    return;
  }
  if (tokens[1] == "binary_little_endian") {
    header.is_ascii = false;
    header.is_bigendian = false;
    return;
  }
  if (tokens[1] == "binary_big_endian") {
    header.is_ascii = false;
    header.is_bigendian = true;
    return;
  }
  throw std::runtime_error("Invalid format \"" + tokens[1] + "\"");
}

void parse_properties(std::istream &file_stream,
                      animesh::PlyFileParser::PlyHeader &header,
                      std::vector<animesh::PlyFileParser::PlyProperty> &properties) {
  while (true) {
    auto p = file_stream.tellg();
    auto line = read_line_skipping_comments(file_stream);
    if (!starts_with(line, "property")) {
      file_stream.seekg(p);
      break;
    }
    auto tokens = split(line, ' ');
    if (tokens.size() != 3 && tokens.size() != 5) {
      throw std::runtime_error("Property line is invalid \"" + line + "\"");
    }

    animesh::PlyFileParser::PlyProperty prop;
    int token_idx = 1;
    if (tokens[token_idx] == "list") {
      prop.is_list = true;
      token_idx++;
      if (find(begin(VALID_LIST_PROPERTY_TYPES), end(VALID_LIST_PROPERTY_TYPES), tokens[token_idx])
          == end(VALID_LIST_PROPERTY_TYPES)) {
        throw std::runtime_error("Property list type \"" + tokens[token_idx] + "\" is not valid");
      }
      prop.list_type = tokens[token_idx];
      token_idx++;
    } else {
      if (find(begin(VALID_PROPERTY_TYPES), end(VALID_PROPERTY_TYPES), tokens[token_idx])
          == end(VALID_PROPERTY_TYPES)) {
        throw std::runtime_error("Property type \"" + tokens[token_idx] + "\" is not valid");
      }
      prop.is_list = false;
    }
    prop.type = tokens[token_idx++];
    prop.name = tokens[token_idx++];
    properties.push_back(prop);
  }
}

void parse_vertex_element(std::istream &file_stream, animesh::PlyFileParser::PlyHeader &header) {
  // Read properties until next non-property tag
  parse_properties(file_stream, header, header.vertex_properties);
}

void parse_face_element(std::istream &file_stream, animesh::PlyFileParser::PlyHeader &header) {
  parse_properties(file_stream, header, header.face_properties);
}

void parse_element(std::istream &file_stream, const std::string &line, animesh::PlyFileParser::PlyHeader &header) {
  using namespace std;

  vector<string> tokens = split(line, ' ');
  /* Expects
    element <name> <count>
   */
  if (tokens.size() != 3) {
    throw std::runtime_error("Invalid element line \"" + line
                                 + "\" expected \"element <name> <count>");
  }

  if (tokens[1] == "vertex") {
    parse_vertex_element(file_stream, header);
    header.num_vertices = stoi(tokens[2]);
    return;
  }

  if (tokens[1] == "face") {
    parse_face_element(file_stream, header);
    header.num_faces = stoi(tokens[2]);
    return;
  }

  throw std::runtime_error("Unknown element type \"" + tokens[1] + "\".");
}

animesh::PlyFileParser::PlyHeader parse_header(std::istream &stream) {
  animesh::PlyFileParser::PlyHeader header{};
  auto line = read_line_skipping_comments(stream);
  if (line != "ply") {
    throw std::runtime_error("Expected \"ply\"");
  }
  // Format
  line = read_line_skipping_comments(stream);
  if (!starts_with(line, "format")) {
    throw std::runtime_error("Expected \"format ... \"");
  }
  parse_format(line, header);

  // Parse elements
  while (true) {
    line = read_line_skipping_comments(stream);
    if (line == "end_header") {
      break;
    }

    if (starts_with(line, "element")) {
      parse_element(stream, line, header);
      continue;
    }
    throw std::runtime_error("Unexpected line \"" + line + "\".");
  }

  return header;
}

void
parse_ascii_vertex_list(std::istream &file_stream,
                        const animesh::PlyFileParser::PlyHeader &header,
                        std::vector<Eigen::Vector3f> vertices) {
}

void parse_ascii_face_list(std::istream &file_stream,
                           const animesh::PlyFileParser::PlyHeader &header,
                           std::vector<std::vector<size_t>> face_indices) {
}

float
read_float(std::istream &stream, bool is_bigendian) {
  float f;
  stream.read((char *) &f, 4);
  if (!is_bigendian) {
    float g;
    const char *src = (char *) &f;
    char *dst = (char *) &g;
    dst[0] = src[3];
    dst[1] = src[2];
    dst[2] = src[1];
    dst[3] = src[0];
    f = g;
  }
  return f;
}

unsigned int
read_int(std::istream &stream, int size, bool is_bigendian) {
  unsigned int i;
  char *pi = (char *) &i;
  switch (size) {
  case 1:stream.read(pi + 3, 1);
    break;
  case 2:stream.read(pi + 2, 2);
    break;
  case 4:stream.read(pi, 4);
    break;
  default:throw std::runtime_error("Invalid size for int " + std::to_string(size));
  }
  if (!is_bigendian) {
    unsigned int j;
    char *dst = (char *) &j;
    dst[0] = pi[3];
    dst[1] = pi[2];
    dst[2] = pi[1];
    dst[3] = pi[0];
    i = j;
  }
  return i;
}

void
parse_binary_vertex_list(std::istream &stream,
                         const animesh::PlyFileParser::PlyHeader &header,
                         std::vector<Eigen::Vector3f> vertices) {
  for (int i = 0; i < header.num_vertices; ++i) {
    float x = read_float(stream, header.is_bigendian);
    float y = read_float(stream, header.is_bigendian);
    float z = 0.0f;
    if (header.vertex_properties.size() == 3) {
      z = read_float(stream, header.is_bigendian);
    }
    vertices.emplace_back(x, y, z);
  }
}

int bytes_for_type(const std::string &type) {
  if (type == "uchar" || type == "char") {
    return 1;
  }

  if (type == "ushort" || type == "short") {
    return 2;
  }

  if (type == "uint" || type == "int") {
    return 4;
  }

  throw std::runtime_error("Unknown list property type " + type);
}

void parse_binary_face_list(std::istream &stream,
                            const animesh::PlyFileParser::PlyHeader &header,
                            std::vector<std::vector<size_t>> face_indices) {
  for (int i = 0; i < header.num_faces; ++i) {
    std::vector<size_t> face;

    // Read num items in list
    int num_bytes_for_list_length = bytes_for_type(header.face_properties[0].list_type);
    unsigned int num_indices_per_face = read_int(stream, num_bytes_for_list_length, header.is_bigendian);
    for (unsigned int j = 0; j < num_indices_per_face; ++j) {
      int num_bytes_for_list_item = bytes_for_type(header.face_properties[0].type);
      size_t idx = read_int(stream, num_bytes_for_list_item, header.is_bigendian);
      face.push_back(idx);
    }
    face_indices.push_back(face);
  }
}

std::pair<std::vector<Eigen::Vector3f>, std::vector<std::vector<std::size_t>>>
animesh::PlyFileParser::parse_stream(std::istream &stream) {
  auto header = parse_header(stream);

  using namespace std;
  using namespace Eigen;

  vector<Vector3f> vertices;
  vector<vector<size_t>> faces;

  if (header.is_ascii) {
    parse_ascii_vertex_list(stream, header, vertices);
    parse_ascii_face_list(stream, header, faces);
  } else {
    parse_binary_vertex_list(stream, header, vertices);
    parse_binary_face_list(stream, header, faces);
  }
  return {vertices, faces};
}

std::pair<std::vector<Eigen::Vector3f>, std::vector<std::vector<std::size_t>>>
animesh::PlyFileParser::parse_file(const std::string &file_name) {
  using namespace std;
  ifstream file_stream{file_name};
  istream &stream = file_stream;
  return parse_stream(stream);
}
}