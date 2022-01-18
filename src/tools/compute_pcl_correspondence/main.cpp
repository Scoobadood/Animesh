//
// Created by Dave Durbin on 13/12/21.
//

#include <vector>
#include <map>
#include <unordered_set>
#include <fstream>

#include <Surfel/PixelInFrame.h>
#include <Correspondence/CorrespondenceIO.h>
#include <Utilities/utilities.h>
#include <Tools/tools.h>
#include <cpd/nonrigid.hpp>
#include <cpd/gauss_transform_fgt.hpp>
#include <spdlog/spdlog.h>

/**
 * Compute correspondences between two point clouds using CPD
 */
void
compute_correspondences(const Eigen::MatrixX3d &from_point_cloud,
                        const Eigen::MatrixX3d &to_point_cloud,
                        std::map<unsigned int, unsigned int> &correspondence) {
  using namespace std;
  using namespace cpd;

  Nonrigid rigid;
  rigid.gauss_transform(move(unique_ptr<GaussTransform>(new GaussTransformFgt())));
  rigid.correspondence(true)
      .max_iterations(20)
      .normalize(false);
  NonrigidResult result = rigid.run(to_point_cloud, from_point_cloud);
  correspondence.clear();
  for (unsigned int i = 0; i < result.correspondence.size(); ++i) {
    correspondence.emplace(i, result.correspondence(i));
  }
}

int main(int argc, const char *argv[]) {
  using namespace std;

  string property_file_name = (argc == 2) ? argv[1] : "animesh.properties";
  Properties properties{property_file_name};

  // Load point clouds
  string pcloud_directory = properties.getProperty("corr-pc-directory");
  string pcloud_regex = properties.getProperty("corr-pc-regex");
  auto pointclouds = load_pointclouds_as_matrices(pcloud_directory, pcloud_regex);

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

  for (unsigned int i = 0; i < pointclouds[0].rows(); ++i) {
    vector<frame_index> path;
    frame_index fi{0, i};
    path.push_back(fi);
    paths.push_back(path);
    path_tail_item_to_path.emplace(fi, i);
  }

  vector<map<unsigned int, unsigned int>> corr;
  for (unsigned int i = 1; i < pointclouds.size(); ++i) {
    unsigned int from_frame = i-1;
    unsigned int to_frame = i;
    spdlog::info("Computing correspondences {} to {}", from_frame, to_frame);

    // Mark all 'to_frame' points as unconsumed
    unsigned int num_target_rows = pointclouds.at(to_frame).rows();
    bool is_consumed[num_target_rows];
    for( auto & b : is_consumed) {
      b = false;
    }

    // Compute correspondences
    corr.emplace_back();
    compute_correspondences(pointclouds[from_frame], pointclouds[to_frame], corr.back());

    // For each correspondence
    for (const auto &entry: corr.back()) {
      unsigned int from_point_idx = entry.first;
      unsigned int to_point_idx = entry.second;

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
    for( unsigned int ti = 0; ti<num_target_rows; ++ti) {
      if( is_consumed[ti] ) {
        continue;
      }

      const frame_index to_key{to_frame, ti};
      vector<frame_index> new_path;
      new_path.push_back(to_key);
      paths.push_back(new_path);
      path_tail_item_to_path.emplace(to_key, paths.size()-1);
    }
  }

  const auto output_paths = pcloud_directory + "/paths.txt";
  ofstream fout{output_paths};
  for (const auto &path: paths) {
    if( path.size() == 1) {
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