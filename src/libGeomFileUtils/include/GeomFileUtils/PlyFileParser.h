//
// Created by Dave Durbin (Old) on 26/9/21.
//

#include <Eigen/Eigen>
#include <istream>

namespace animesh {

class PlyFileParser {
public:
  struct PlyProperty {
    std::string name;
    std::string type;
    bool is_list{};
    std::string list_type;
  };

  struct PlyHeader {
    bool is_ascii;
    bool is_bigendian;
    int version;
    int num_vertices;
    int num_faces;
    std::vector<PlyProperty> vertex_properties;
    std::vector<PlyProperty> face_properties;
  };

  /**
   * Parse an OBJ file and return only points and faces
   * @param file_name The name of the file.
   * @return A pair of vectors containing the points and face vertex indices.
   */
  static std::pair<std::vector<Eigen::Vector3f>, std::vector<std::vector<std::size_t>>>
  parse_file(const std::string &file_name);

  static std::pair<std::vector<Eigen::Vector3f>, std::vector<std::vector<std::size_t>>>
  parse_stream(std::istream &stream);

};

animesh::PlyFileParser::PlyHeader parse_header(std::istream &stream);
}