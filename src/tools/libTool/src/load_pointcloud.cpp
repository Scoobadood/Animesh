
#include <tools.h>
#include <regex>
#include <FileUtils/FileUtils.h>
#include <CommonUtilities/split.h>

/**
 * Load pointcloud from file
 */
std::vector<Eigen::Vector3f>
load_vec3f_from_file(const std::string &vec_filename) {
  using namespace std;
  using namespace Eigen;

  vector<Vector3f> vectors;

  process_file_by_lines(vec_filename, [&vectors](const std::string &line) {
    auto vertex = string_to_vec3f(line);
    vectors.emplace_back(vertex);
  });
  return vectors;
};

/**
 * Given a regex matching one or more files, which should include one capture group that identifies the frame
 * number. This is assumed to be an integer and will be used as the key in the returned map.
 * Read all vectors from these files and return in a map.
 */
std::map<unsigned int, std::vector<Eigen::Vector3f>>
load_vec3s_from_directory(const std::string &directory, const std::string &pattern) {
  using namespace std;
  using namespace Eigen;

  regex p{pattern};
  vector<string> vec3_files;
  files_in_directory(directory, vec3_files, [&p](std::string file_name) {
    using namespace std;
    transform(file_name.begin(), file_name.end(), file_name.begin(), ::tolower);
    smatch matches;
    return regex_search(file_name, matches, p);
  });
  sort(vec3_files.begin(), vec3_files.end());

  map<unsigned int, vector<Vector3f>> vec3_by_file_index;
  for (const auto &file_name: vec3_files) {
    smatch matches;
    string lc_filename = file_name;
    transform(lc_filename.begin(), lc_filename.end(), lc_filename.begin(), ::tolower);
    if (!regex_search(lc_filename, matches, p)) {
      throw runtime_error("Invalid vec3 filename " + file_name);
    }
    // 0 is the whole string, 1 is the frame
    unsigned int file_idx = stoi(matches[1].str());
    auto vec3f = load_vec3f_from_file(file_name);
    vec3_by_file_index.emplace(file_idx, vec3f);
  }

  return vec3_by_file_index;
}

/**
 * Load all pointclouds from a directory given a regex pattern.
 * The regex should include one capture group that identifies the frame
 * number. This is assumed to be an integer and will be used as the key in the returned map.
 */
std::map<unsigned int, Eigen::MatrixX3d>
load_vec3f_from_directory_as_matrices(const std::string &directory, const std::string &pattern) {
  using namespace std;
  using namespace Eigen;

  auto vec3fs = load_vec3s_from_directory(directory, pattern);

  map<unsigned int, MatrixX3d> clouds;
  for( const auto & vec3f : vec3fs) {
    Eigen::MatrixX3d m{vec3f.second.size(), 3};
    unsigned int row = 0;
    for( const auto & vec : vec3f.second) {
      m(row, 0) = vec.x();
      m(row, 1) = vec.y();
      m(row, 2) = vec.z();
      row++;
    }
    clouds.emplace(vec3f.first, m);
  }
  return clouds;
}