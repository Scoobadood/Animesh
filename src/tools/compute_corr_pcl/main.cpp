//
// Created by Dave Durbin on 7/3/2022.
//

#include <Eigen/Core>
#include <pcl/point_types.h>
#include <pcl/point_cloud.h>
#include <pcl/registration/correspondence_estimation_normal_shooting.h>
#include <vector>
#include <fstream>
#include <spdlog/spdlog.h>
#include <Properties/Properties.h>
#include <Tools/tools.h>

pcl::PointCloud<pcl::PointNormal>::Ptr
convert_matrix_to_point_cloud(const Eigen::MatrixXd &source_cloud,
                              const Eigen::MatrixXd &source_normals) {
  using namespace pcl;

  PointCloud<PointNormal>::Ptr cloud(new PointCloud<PointNormal>);
  for (int i = 0; i < source_cloud.rows(); i++) {
    pcl::PointNormal temp;
    temp.x = source_cloud.row(i).x();
    temp.y = source_cloud.row(i).y();
    temp.z = source_cloud.row(i).z();
    temp.normal_x = source_normals.row(i).x();
    temp.normal_y = source_normals.row(i).y();
    temp.normal_z = source_normals.row(i).z();
    cloud->push_back(temp);
  }
  return cloud;
}

/**
 * Compute correspondences between two point clouds using PCL
 */
void
compute_correspondences(const Eigen::MatrixX3d &from_point_cloud,
                        const Eigen::MatrixX3d &to_point_cloud,
                        const Eigen::MatrixX3d &from_normals,
                        const Eigen::MatrixX3d &to_normals,
                        std::map<unsigned int, unsigned int> &correspondence) {
  using namespace std;
  using namespace pcl;

  // ... read or fill in source and target
  registration::CorrespondenceEstimationNormalShooting<PointNormal, PointNormal, PointNormal> est;

  const auto source = convert_matrix_to_point_cloud(from_point_cloud, from_normals);
  est.setInputSource(source);
  est.setSourceNormals(source);

  const auto target = convert_matrix_to_point_cloud(to_point_cloud, to_normals);
  est.setInputTarget(target);

  // Test the first 10 correspondences for each point in source, and return the best
  est.setKSearch(10);

  // Determine all correspondences
  pcl::Correspondences all_correspondences;
  est.determineCorrespondences(all_correspondences);

  for( const auto & corr : all_correspondences) {
    correspondence.emplace( corr.index_query, corr.index_match);
  }
}

int main(int argc, const char *argv[]) {
  using namespace std;

  string property_file_name = (argc == 2) ? argv[1] : "animesh.properties";
  Properties properties{property_file_name};

  // Load point clouds
  string pcloud_directory = properties.getProperty("corr-pc-directory");

  string pcloud_regex = properties.getProperty("corr-pc-regex");
  auto pointclouds = load_vec3f_from_directory_as_matrices(pcloud_directory, pcloud_regex);

  string normal_regex = properties.getProperty("corr-norm-regex");
  auto normals = load_vec3f_from_directory_as_matrices(pcloud_directory, normal_regex);

  struct frame_index {
    unsigned int frame;
    unsigned int idx;
    bool operator<(const frame_index &other) const {
      if (frame < other.frame) return true;
      if (frame > other.frame) return false;
      if (idx < other.idx) return true;
      return false;
    }
    frame_index(unsigned int f, unsigned int i) {
      frame = f;
      idx = i;
    }
  };

  // Paths are vectors of (frame,index) pairs
  vector<vector<frame_index>> paths;

  // This maps from the item at the tail of the path into the paths vector
  map<frame_index, unsigned int> path_tail_item_to_path;

  // Create a new path starting at each point in the first frame
  for (unsigned int i = 0; i < pointclouds.at(0).rows(); ++i) {
    vector<frame_index> path;
    frame_index fi{0, i};
    path.push_back(fi);
    paths.push_back(path);
    path_tail_item_to_path.emplace(fi, i);
  }

  vector<map<unsigned int, unsigned int>> corr;
  for (unsigned int i = 1; i < pointclouds.size(); ++i) {
    unsigned int from_frame = i - 1;
    unsigned int to_frame = i;
    spdlog::info("Computing correspondences {} to {}", from_frame, to_frame);

    // Mark all 'to_frame' points as unconsumed
    unsigned int num_target_rows = pointclouds.at(to_frame).rows();
    bool is_consumed[num_target_rows];
    for (auto &b: is_consumed) {
      b = false;
    }

    // Compute correspondences
    corr.emplace_back();
    compute_correspondences(pointclouds[from_frame],
                            pointclouds[to_frame],
                            normals[from_frame],
                            normals[to_frame],
                            corr.back());

    // For each correspondence
    int ii = 0;
    for (const auto &entry: corr.back()) {
      unsigned int from_point_idx = entry.first;
      unsigned int to_point_idx = entry.second;
      spdlog::info("{}: {}, {}, {}, {}, {}, {} % {}", ii,
                   pointclouds[from_frame](from_point_idx, 0),
                   pointclouds[from_frame](from_point_idx, 1),
                   pointclouds[from_frame](from_point_idx, 2),
                   pointclouds[to_frame](to_point_idx, 0),
                   pointclouds[to_frame](to_point_idx, 1),
                   pointclouds[to_frame](to_point_idx, 2),
                   (entry.first == entry.second) ? "" : "x"
      );
      ii++;
      // Some other path includes this point already. Skip it.
      if (is_consumed[to_point_idx]) {
        continue;
      }

      // Get the path that contains the from_point_idx
      const frame_index from_key{from_frame, from_point_idx};
      auto path_index = path_tail_item_to_path.at(from_key);

      const frame_index to_key{to_frame, to_point_idx};
      paths.at(path_index).push_back(to_key);
      path_tail_item_to_path.emplace(to_key, path_index);
      is_consumed[to_point_idx] = true;
    }

    // Now add any unconsumed points as the start of a new path
    for (unsigned int ti = 0; ti < num_target_rows; ++ti) {
      if (is_consumed[ti]) {
        continue;
      }

      const frame_index to_key{to_frame, ti};
      vector<frame_index> new_path;
      new_path.push_back(to_key);
      paths.push_back(new_path);
      path_tail_item_to_path.emplace(to_key, paths.size() - 1);
    }
  }

  const auto output_paths = pcloud_directory + "/paths.txt";
  ofstream fout{output_paths};
  for (const auto &path: paths) {
    if (path.size() == 1) {
      continue;
    }
    for (int i = 0; i < path.size(); ++i) {
      fout << "(" << path.at(i).frame << " " << path.at(i).idx << ")";
      if (i < path.size() - 1) {
        fout << ",";
      }
    }
    fout << endl;
  }
  fout.close();
}