
#include <tools.h>
#include <regex>
#include <FileUtils/FileUtils.h>
#include <CommonUtilities/split.h>

/**
 * Load pointcloud from file
 */
std::vector<Eigen::Vector3f>
load_pointcloud(const std::string &pointcloud_filename) {
  using namespace std;
  using namespace Eigen;

  vector<Vector3f> pointcloud;

  process_file_by_lines(pointcloud_filename, [&pointcloud](const std::string &line) {
    auto vertex = string_to_vec3f(line);
    pointcloud.emplace_back(vertex);
  });
  return pointcloud;
};

/**
 * Load all pointclouds from a directory given a regex pattern.
 * The regex should include one capture group that identifies the frame
 * number. This is assumed to be an integer and will be used as the key in the returned map.
 */
std::map<unsigned int, std::vector<Eigen::Vector3f>>
load_pointclouds(const std::string &directory, const std::string &pattern) {
  using namespace std;
  using namespace Eigen;

  regex p{pattern};
  vector<string> pointcloud_files;
  files_in_directory(directory, pointcloud_files, [&p](std::string file_name) {
    using namespace std;
    transform(file_name.begin(), file_name.end(), file_name.begin(), ::tolower);
    smatch matches;
    return regex_search(file_name, matches, p);
  });
  sort(pointcloud_files.begin(), pointcloud_files.end());

  map<unsigned int, vector<Vector3f>> pointclouds_by_frame;
  for (const auto &file_name: pointcloud_files) {
    smatch matches;
    string lc_filename = file_name;
    transform(lc_filename.begin(), lc_filename.end(), lc_filename.begin(), ::tolower);
    if (!regex_search(lc_filename, matches, p)) {
      throw runtime_error("Invalid pointcloud filename " + file_name);
    }
    // 0 is the whole string, 1 is the frame
    unsigned int frameIdx = stoi(matches[1].str());
    auto pointcloud = load_pointcloud(file_name);
    pointclouds_by_frame.emplace(frameIdx, pointcloud);
  }

  return pointclouds_by_frame;
}

/**
 * Load all pointclouds from a directory given a regex pattern.
 * The regex should include one capture group that identifies the frame
 * number. This is assumed to be an integer and will be used as the key in the returned map.
 */
std::map<unsigned int, Eigen::MatrixX3d>
load_pointclouds_as_matrices(const std::string &directory, const std::string &pattern) {
  using namespace std;
  using namespace Eigen;

  auto pointclouds = load_pointclouds(directory, pattern);

  map<unsigned int, MatrixX3d> clouds;
  for( const auto & pointcloud : pointclouds) {
    Eigen::MatrixX3d m{pointcloud.second.size(), 3};
    unsigned int row = 0;
    for( const auto & vertex : pointcloud.second) {
      m(row, 0) = vertex.x();
      m(row, 1) = vertex.y();
      m(row, 2) = vertex.z();
      row++;
    }
    clouds.emplace( pointcloud.first, m);
  }
  return clouds;
}
