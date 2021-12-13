//
// Created by Dave Durbin on 13/12/21.
//

#include <vector>
#include <Eigen/Eigen>

#include <Surfel/PixelInFrame.h>
#include <Correspondence/CorrespondenceCompute.h>
#include <Utilities/utilities.h>
#include <Tools/tools.h>

/**
 * Compute correspondences between two point clouda using CPD
 *
 * @param level
 * @param frame
 * @param correspondence
 * @param file_name_template
 */
void
compute_correspondences(const Eigen::MatrixX3d &from_point_cloud,
                        const Eigen::MatrixX3d &to_point_cloud,
                        std::map<unsigned int, unsigned int> &correspondence) {
  using namespace std;
  using namespace cpd;

  Rigid rigid;
  rigid.gauss_transform(std::move(
      std::unique_ptr<cpd::GaussTransform>(new cpd::GaussTransformFgt())));
  rigid.correspondence(true);
  rigid.normalize(false);
  cpd::RigidResult result = rigid.run(to_point_cloud, from_point_cloud);
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
  string pcloud_regex = properties.getProperty( "corr-pc-regex");
  auto pointclouds = load_pointclouds(pcloud_directory, pcloud_regex);
}