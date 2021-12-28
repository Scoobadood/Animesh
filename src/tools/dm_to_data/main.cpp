//
// Created by Dave Durbin on 13/12/21.
//

/*
 * Generate deptyh and normal maps from the OBJ files
 */
#include <Utilities/utilities.h>
#include <DepthMap/DepthMap.h>
#include <Properties/Properties.h>
#include <Surfel/Pixel.h>
#include <Surfel/PixelInFrame.h>
#include <spdlog/spdlog.h>
#include <fstream>

/**
 * Print usage instructions if the number of arguments is wrong or arguments are inconsistent
 *
 * @param prog_name The name of the program with path.
 */
void usage(const std::string &prog_name) {
  std::cout << "usage: " << prog_name << " [source_directory]" << std::endl;
  exit(-1);
}

/**
 * Return a vector of Pixel coordinates for only those pixels in a frame which are 'valid'
 * Valid in this context means that they have non-zero depth and a well defined normal.
 * In fact having a well defined normal means that they don't have zero depth.
 * @param depth_map
 * @return
 */
static std::vector<Pixel>
get_valid_pixels_for_depth_map(const DepthMap &depth_map) {
  using namespace std;

  // Filter out invalid points (points which have no normals is the thing)
  vector<Pixel> valid_pixels;
  for (unsigned int y = 0; y < depth_map.height(); ++y) {
    for (unsigned int x = 0; x < depth_map.width(); ++x) {
      if (depth_map.is_normal_defined(x, y)) {
        valid_pixels.emplace_back(x, y);
      }
    }
  }
  return valid_pixels;
}

/*
 * Given a camera, a set of N X,Y coordinates and a depth map, generate camera space 3D coordinates
 * and return as an Nx3 matrix.
 */
static Eigen::MatrixX3d
project_pixels_to_point_cloud(const std::vector<Pixel> &pixels,
                              const Camera &camera,
                              const DepthMap &depth_map) {
  using namespace std;

  Eigen::MatrixX3d m{pixels.size(), 3};

  unsigned int row = 0;
  for (const auto &pixel : pixels) {
    float depth = depth_map.depth_at(pixel.x, pixel.y);

    float xyz[3];
    camera.to_world_coordinates(pixel.x, pixel.y, depth, xyz);
    m(row, 0) = xyz[0];
    m(row, 1) = xyz[1];
    m(row, 2) = xyz[2];
    row++;
  }

  return m;
}


/**
 * Write a point_cloud to a text file.
 * Each row contains the X,Y,Z coordinates of a point.
 *
 * @param pointcloud
 * @param file_name
 */
void
save_point_cloud_to_file(Eigen::MatrixX3d point_cloud, const std::string &file_name) {
  using namespace std;

  ofstream file{file_name};
  for (unsigned int row = 0; row < point_cloud.rows(); ++row) {
    file << point_cloud(row, 0) << ", " << point_cloud(row, 1) << ", " << point_cloud(row, 2) << endl;
  }
}

void
save_point_clouds_to_file(const std::vector<std::vector<Eigen::MatrixX3d>> &point_clouds,
                          const std::string &file_name_template) {
  using namespace std;

  for (unsigned int level = 0; level < point_clouds.size(); ++level) {
    for (unsigned int frame = 0; frame < point_clouds.at(level).size(); ++frame) {
      string file_name = file_name_from_template_level_and_frame(file_name_template, level, frame);
      save_point_cloud_to_file(point_clouds.at(level).at(frame), file_name);
    }
  }
}

void
save_pifs_to_file(const std::vector<Pixel>& pixels, const std::string &file_name) {
  using namespace std;

  ofstream file{file_name};
  for (const auto &pixel : pixels) {
    file << "(" << pixel.x << ", " << pixel.y << ")" << endl;
  }
}


void
save_pifs_to_file(const std::vector<std::vector<std::vector<Pixel>>>& valid_pixels_for_levels,
                  const std::string &file_name_template) {
  using namespace std;

  for (unsigned int level = 0; level < valid_pixels_for_levels.size(); ++level) {
    for (unsigned int frame = 0; frame < valid_pixels_for_levels.at(level).size(); ++frame) {
      string file_name = file_name_from_template_level_and_frame(file_name_template, level, frame);
      save_pifs_to_file(valid_pixels_for_levels.at(level).at(frame), file_name);
    }
  }
}

/**
 * Save normals to the given file
 *
 * @param valid_pixels
 * @param depth_map
 * @param level
 * @param frame
 * @param file_name_template
 */
void save_normals(const std::vector<Pixel> &valid_pixels,
                  const DepthMap &depth_map,
                  const std::string &file_name) {
  using namespace std;

  ofstream file{file_name};
  for (const auto &pixel : valid_pixels) {
    auto normal = depth_map.normal_at(pixel.x, pixel.y);
    file << normal.x << ", " << normal.y << ", " << normal.z << endl;
  }
}

/**
 * Given a vector (per frame) of derpth_maps
 * Return a vectror (per frame) of valid pixels.
 * @param depth_maps
 * @return
 */
std::vector<std::vector<Pixel>>
get_valid_pixels_for_depth_maps(const std::vector<DepthMap> &depth_maps) {
  using namespace std;

  int frame_index = 0;
  vector<vector<Pixel>> valid_pixels;
  for (const auto &depth_map : depth_maps) {
    vector<Pixel> valid_pixels_in_frame = get_valid_pixels_for_depth_map(depth_map);
    valid_pixels.push_back(valid_pixels_in_frame);

    frame_index++;
  }
  return valid_pixels;
}

/**
 * Given a vector (per level) of vectors (per frame) of depthmaps
 * return a vector (per level) or vectors (per frame) of pixels
 * representing thre valid pixels.
 * @param depth_map_hierarchy
 * @return
 */
std::vector<std::vector<std::vector<Pixel>>>
get_valid_pixels_for_all_levels(const std::vector<std::vector<DepthMap>> &depth_map_hierarchy) {
  using namespace std;

  vector<vector<vector<Pixel>>> valid_pixels_for_levels;

  unsigned int level = 1;
  for (const auto &depth_maps : depth_map_hierarchy) {
    cout << "Extracting valid pixels for level : " << level << endl;
    valid_pixels_for_levels.push_back(get_valid_pixels_for_depth_maps(depth_maps));
    ++level;
  }
  return valid_pixels_for_levels;
}

/**
 * Given a vector (per frame) of pixels
 * and a vector (per frame) of cameras
 * and a vector (per frame) of depth maps
 * Return a vector (per frame) of 3D points they represent
 */
std::vector<Eigen::MatrixX3d>
project_pixels_to_point_clouds(const std::vector<std::vector<Pixel>> &pixels_by_frame,
                               const std::vector<Camera> &cameras_by_frame,
                               const std::vector<DepthMap> &depth_maps_by_frame) {
  using namespace std;

  vector<Eigen::MatrixX3d> point_clouds;
  for (unsigned int frame = 0; frame < cameras_by_frame.size(); ++frame) {
    point_clouds.push_back(project_pixels_to_point_cloud(pixels_by_frame.at(frame),
                                                         cameras_by_frame.at(frame),
                                                         depth_maps_by_frame.at(frame)));
  }
  return point_clouds;
}

/**
 * Given a vector (per level) of vectors (per frame) of Pixels
 * plus associated depth maps and cameras
 * return a vector (per level) of vectors (per frame) of Eigen X3 matrices containing
 * the 3D points correspondig to those pixels.
 */
std::vector<std::vector<Eigen::MatrixX3d>>
project_pixels_to_point_clouds(const std::vector<std::vector<std::vector<Pixel>>> &pixels_by_level_and_frame,
                               const std::vector<Camera> &cameras_by_frame,
                               const std::vector<std::vector<DepthMap>> &depth_maps_by_level_and_frame) {
  using namespace std;

  auto num_levels = depth_maps_by_level_and_frame.size();

  vector<vector<Eigen::MatrixX3d>> point_clouds;
  for (unsigned int level = 0; level < num_levels; ++level) {
    cout << "Computing pointclouds for level : " << level << endl;

    size_t image_width = depth_maps_by_level_and_frame.at(level).at(0).width();
    size_t image_height = depth_maps_by_level_and_frame.at(level).at(0).height();

    // Set the image resolution for all cameras at this level
    vector<Camera> cameras_for_level;
    for (auto camera : cameras_by_frame) {
      camera.set_image_size(image_width, image_height);
      cameras_for_level.push_back(camera);
    }

    point_clouds.push_back(project_pixels_to_point_clouds(pixels_by_level_and_frame.at(level),
                                                          cameras_for_level,
                                                          depth_maps_by_level_and_frame.at(level)));
  }
  return point_clouds;
}

/**
 * Save the normals for each pixel.
 * @param pixels_for_level_and_frame
 * @param depth_map_hierarchy
 * @param normal_file_template
 */
void
save_normals_to_file(const std::vector<std::vector<std::vector<Pixel>>> &pixels_for_level_and_frame,
                     const std::vector<std::vector<DepthMap>> &depth_map_hierarchy,
                     const std::string &normal_file_template) {
  for (unsigned int level = 0; level < pixels_for_level_and_frame.size(); ++level) {
    for (unsigned int frame = 0; frame < pixels_for_level_and_frame.at(level).size(); ++frame) {

      save_normals(pixels_for_level_and_frame.at(level).at(frame), depth_map_hierarchy.at(level).at(frame),
                   file_name_from_template_level_and_frame(normal_file_template, level, frame));
    }
  }
}

// Save paths to file
void save_paths_to_file(const std::vector<std::vector<PixelInFrame>> &paths,
                        const std::string &file_name) {
  using namespace std;

  ofstream file{file_name};
  for (const auto &path: paths) {
    for (const auto &pif : path) {
      file << "( " << pif.frame << ", " << pif.pixel.x << ", " << pif.pixel.y << ") ";
    }
    file << endl;
  }
  file << endl;
}

int main(int argc, char *argv[]) {
  using namespace std;
  using namespace std::chrono;

  string property_file_name = (argc == 2) ? argv[1] : "animesh.properties";
  Properties properties{property_file_name};

  // Load depth maps
  vector<DepthMap> depth_maps = load_depth_maps(properties);
  size_t num_frames = depth_maps.size();

  // Load cameras
  string camera_file_template = properties.getProperty("camera-file-template");
  vector<Camera> cameras = load_cameras(camera_file_template, num_frames);

  // Construct the hierarchy: number of levels as specified in properties.
  vector<vector<DepthMap>> depth_map_hierarchy = create_depth_map_hierarchy(properties, depth_maps, cameras);

  // Vector of level, frame, pixel index
  vector<vector<vector<Pixel>>> valid_pixels_for_levels = get_valid_pixels_for_all_levels(depth_map_hierarchy);
  if (properties.getBooleanProperty("save-valid-pifs")) {
    save_pifs_to_file(valid_pixels_for_levels, properties.getProperty("pif-file-template"));
  }

  // Project valid pixels into point clouds
  vector<vector<Eigen::MatrixX3d>> point_clouds_for_all_levels =
      project_pixels_to_point_clouds(valid_pixels_for_levels, cameras, depth_map_hierarchy);

  // Save point clouds if requested
  if (properties.getBooleanProperty("save-point-clouds")) {
    save_point_clouds_to_file(point_clouds_for_all_levels, properties.getProperty("point-cloud-file-template"));
  }

  // Save normals if requested
  if (properties.getBooleanProperty("save-normals")) {
    save_normals_to_file(valid_pixels_for_levels, depth_map_hierarchy,
                         properties.getProperty("normal-file-template"));
  }

  return 0;
}