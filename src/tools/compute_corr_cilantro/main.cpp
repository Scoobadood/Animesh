//
// Created by Dave Durbin on 13/12/21.
//

#include <vector>
#include <map>
#include <unordered_set>
#include <fstream>

#include <cilantro/registration/icp_common_instances.hpp>
#include <cilantro/utilities/point_cloud.hpp>

#include <Surfel/PixelInFrame.h>
#include <Correspondence/CorrespondenceIO.h>
#include <Utilities/utilities.h>
#include <Tools/tools.h>
#include <spdlog/spdlog.h>
#include <cpd/rigid.hpp>


/**
 * Load all pointclouds from a directory given a regex pattern.
 * The regex should include one capture group that identifies the frame
 * number. This is assumed to be an integer and will be used as the key in the returned map.
 */
std::vector<cilantro::PointCloud3f>
load_pcls(const std::string &directory, const std::string &pattern) {
  using namespace std;
  using namespace Eigen;
  using namespace cilantro;

  auto pointclouds = load_vec3s_from_directory(directory, pattern);
  size_t start_index[pointclouds.size()];
  size_t end_index[pointclouds.size()];

  int pci = 0;
  int idx = 0;
  vector<float> coords;
  for( const auto & pc : pointclouds) {
    start_index[pci] = coords.size();
    for( const auto & p : pc.second ) {
      coords.push_back(p.x());
      coords.push_back(p.y());
      coords.push_back(p.z());
    }
    end_index[pci] = coords.size();
    pci++;
  }

  vector<PointCloud3f> point_clouds;
  for( int i=0; i < pointclouds.size(); ++i) {
    auto start = start_index[i];
    const int count = (end_index[i] - start_index[i] - 1);
    auto first = coords.begin() + start;
    auto last = coords.begin() + start + count;
    vector<float> newVec(first, last);
    ConstVectorSetMatrixMap3f pp{newVec};
    ConstVectorSetMatrixMap3f nn = pp;
    ConstVectorSetMatrixMap3f cc = pp;

    point_clouds.emplace_back(pp, nn, cc);
  }
  return point_clouds;
}


/**
 * Compute correspondences between two point clouds using CPD
 */
void
compute_correspondences(const cilantro::PointCloud3f &from_point_cloud,
                        const cilantro::PointCloud3f &to_point_cloud,
                        std::map<unsigned int, unsigned int> &correspondence) {
  using namespace std;
  using namespace cilantro;

  // Example 1: Compute a sparsely supported warp field (compute transformations for a sparse set of control nodes)
  // Neighborhood parameters
  float control_res = 0.025f;
  float src_to_control_sigma = 0.5f*control_res;
  float regularization_sigma = 3.0f*control_res;

  float max_correspondence_dist_sq = 0.02f*0.02f;

  // Get a sparse set of control nodes by downsampling
  cilantro::VectorSet<float,3> control_points = cilantro::PointsGridDownsampler3f(from_point_cloud.points, control_res).getDownsampledPoints();
  cilantro::KDTree<float,3> control_tree(control_points);

  // Find which control nodes affect each point in src
  cilantro::NeighborhoodSet<float> src_to_control_nn = control_tree.search(from_point_cloud.points, cilantro::KNNNeighborhoodSpecification<>(4));

  // Get regularization neighborhoods for control nodes
  cilantro::NeighborhoodSet<float> regularization_nn = control_tree.search(control_points, cilantro::KNNNeighborhoodSpecification<>(8));

//    cilantro::SimpleCombinedMetricSparseAffineWarpFieldICP3f icp(dst.points, dst.normals, src.points, src_to_control_nn, control_points.cols(), regularization_nn);
  cilantro::SimpleCombinedMetricSparseRigidWarpFieldICP3f icp(to_point_cloud.points, to_point_cloud.normals, from_point_cloud.points, src_to_control_nn, control_points.cols(), regularization_nn);

  // Parameter setting
  icp.correspondenceSearchEngine().setMaxDistance(max_correspondence_dist_sq);
  icp.controlWeightEvaluator().setSigma(src_to_control_sigma);
  icp.regularizationWeightEvaluator().setSigma(regularization_sigma);

  icp.setMaxNumberOfIterations(15).setConvergenceTolerance(2.5e-3f);
  icp.setMaxNumberOfGaussNewtonIterations(1).setGaussNewtonConvergenceTolerance(5e-4f);
  icp.setMaxNumberOfConjugateGradientIterations(500).setConjugateGradientConvergenceTolerance(1e-5f);
  icp.setPointToPointMetricWeight(0.0f).setPointToPlaneMetricWeight(1.0f).setStiffnessRegularizationWeight(200.0f);
  icp.setHuberLossBoundary(1e-2f);

  auto tf_est = icp.estimate().getDenseWarpField();

  std::cout << "Iterations performed: " << icp.getNumberOfPerformedIterations() << std::endl;
  std::cout << "Has converged: " << icp.hasConverged() << std::endl;

  auto residuals = icp.getResiduals();

}

int main(int argc, const char *argv[]) {
  using namespace std;

  string property_file_name = (argc == 2) ? argv[1] : "animesh.properties";
  Properties properties{property_file_name};

  // Load point clouds
  string pcloud_directory = properties.getProperty("corr-pc-directory");
  string pcloud_regex = properties.getProperty("corr-pc-regex");
  auto pointclouds = load_pcls(pcloud_directory, pcloud_regex);

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
  for (unsigned int i = 0; i < pointclouds.at(0).points.rows(); ++i) {
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
    unsigned int num_target_rows = pointclouds.at(to_frame).points.rows();
    bool is_consumed[num_target_rows];
    for (auto &b: is_consumed) {
      b = false;
    }

    // Compute correspondences
    corr.emplace_back();
    compute_correspondences(pointclouds[from_frame], pointclouds[to_frame], corr.back());

    // For each correspondence
    int ii=0;
    for (const auto &entry: corr.back()) {
      unsigned int from_point_idx = entry.first;
      unsigned int to_point_idx = entry.second;
      spdlog::info("{}: {}, {}, {}, {}, {}, {} % {}",ii,
                   pointclouds[from_frame].points(from_point_idx, 0),
                   pointclouds[from_frame].points(from_point_idx, 1),
                   pointclouds[from_frame].points(from_point_idx, 2),
                   pointclouds[to_frame].points(to_point_idx, 0),
                   pointclouds[to_frame].points(to_point_idx, 1),
                   pointclouds[to_frame].points(to_point_idx, 2),
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