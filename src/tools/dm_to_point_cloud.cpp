//
// Created by Dave Durbin on 12/11/19.
// Tool to generate a point cloud file from an input depth map.
// The file is written as a series of X,Y,Z rows in camera space
// Requires that we have a depth map
// Requires that we specify an output file name
// This was developed to run offline CPD code which uses cpd::path_to_matrix to load clouds


#include <vector>
#include <iostream>
#include <fstream>

#include <DepthMap/DepthMap.h>
#include <Properties/Properties.h>
#include <Camera/Camera.h>
#include "../mesher/types.h"
#include "../mesher/correspondences_io.h"
#include "../mesher/utilities.h"

#include <cpd/gauss_transform_fgt.hpp>
#include <cpd/rigid.hpp>
#include <Eigen/Dense>
/**
 * Construct the save file name from a given template and level
 *
 */
std::string
file_name_from_template_and_level(const std::string &file_name_template, unsigned int level) {
    ssize_t bufsz = snprintf(nullptr, 0, file_name_template.c_str(), level);
    char file_name[bufsz + 1];
    snprintf(file_name, bufsz + 1, file_name_template.c_str(), level);
    return file_name;
}

/**
 * Construct the save file name from a given template and level and frame
 *
 */
std::string
file_name_from_template_level_and_frame(const std::string &file_name_template, unsigned int level, unsigned int frame) {
    // We expect 2x %2dL
    ssize_t bufsz = snprintf(nullptr, 0, file_name_template.c_str(), level, frame);
    char file_name[bufsz + 1];
    snprintf(file_name, bufsz + 1, file_name_template.c_str(), level, frame);
    return file_name;
}

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
    for (unsigned int r = 0; r < depth_map.rows(); ++r) {
        for (unsigned int c = 0; c < depth_map.cols(); ++c) {
            if( depth_map.is_normal_defined(r, c)) {
                valid_pixels.emplace_back(c, r);
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

    struct Point3D {
        float x;
        float y;
        float z;

        Point3D(float x, float y, float z) : x(x), y(y), z(z) {};
    };

    // Filter out zero depth points
    vector<Point3D> valid_points;
    for (const auto &pixel : pixels) {
        float depth = depth_map.depth_at(pixel.y, pixel.x);
        // backproject using camera settings
        float x, y, z;
        backproject(camera, pixel.x, pixel.y, depth, &x, &y, &z);
        valid_points.emplace_back(x, y, z);
    }

    // Convert remaining to a Nx3 matrix
    Eigen::MatrixX3d m{valid_points.size(), 3};
    int row = 0;
    for (const auto &p : valid_points) {
        m(row, 0) = p.x;
        m(row, 1) = p.y;
        m(row, 2) = p.z;
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
save_point_cloud_to_file(Eigen::MatrixX3d point_cloud, const std::string& file_name) {
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

/**
 * Save normals to the given file
 *
 * @param valid_pixels
 * @param depth_map
 * @param level
 * @param frame
 * @param file_name_template
 */
void save_normals(const std::vector<Pixel>& valid_pixels,
        const DepthMap& depth_map,
        const std::string& file_name ) {
    using namespace std;

    ofstream file{file_name};
    for (const auto& pixel : valid_pixels) {
        auto normal = depth_map.normal_at(pixel.x, pixel.y);
        file << normal.x << ", " << normal.y << ", " << normal.z << endl;
    }
}

/**
 * Compute correspondences between two point clouda using CPD
 *
 * @param level
 * @param frame
 * @param correspondence
 * @param file_name_template
 */
void
compute_correspondences(const Eigen::MatrixX3d& from_point_cloud,
                        const Eigen::MatrixX3d& to_point_cloud,
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

/**
 * Given pixels and point clouds for each frame in a level
 * Return the paths for points in that level.
 * @param pixels_by_frame
 * @param point_clouds_by_frame
 * @return
 */
std::vector<std::vector<PixelInFrame>>
compute_correspondence_paths_for_level(const std::vector<std::vector<Pixel>>& pixels_by_frame,
                                       const std::vector<Eigen::MatrixX3d>& point_clouds_by_frame) {
    using namespace std;

    map<PixelInFrame, unsigned int> pif_to_path;
    multimap<unsigned int, PixelInFrame> path_to_pifs;

    // For each frame find correspondences to the next frame
    for( unsigned int frame = 0; frame < pixels_by_frame.size() - 1; ++frame ) {
        map<unsigned int, unsigned int> correspondences;
        cout << "  running CPD for from frame : " << frame << endl;
        compute_correspondences(point_clouds_by_frame.at(frame), point_clouds_by_frame.at(frame+1), correspondences);

        unsigned int new_path_id = 1;
        //  For each pixel in from frame
        cout << "  building paths" << endl;
        for( auto& correspondence : correspondences) {

            PixelInFrame from_pif{pixels_by_frame.at(frame).at(correspondence.first), frame};
            PixelInFrame to_pif{pixels_by_frame.at(frame+1).at(correspondence.second), frame+1};

            // First corresponds to second.
            // If first exists in any path, we add to that path
            auto it = pif_to_path.find(from_pif);
            if( it != pif_to_path.end() ) {
                path_to_pifs.insert(make_pair(it->second, to_pif));
                pif_to_path.insert(make_pair(to_pif, it->second));
            } else {
                // Otherwise, we start a new path
                path_to_pifs.insert(make_pair(new_path_id, to_pif));
                pif_to_path.insert(make_pair(to_pif, new_path_id));
                ++new_path_id;
            }
        }
    }

    // Convert correspondence paths to groups
    vector<vector<PixelInFrame>> paths;
    for (auto it1 = path_to_pifs.begin(), it2 = it1, end = path_to_pifs.end(); it1 != end; it1 = it2){
        paths.emplace_back();
        for ( ; it1->first == it2->first; ++it2){
            paths.back().push_back(it2->second);
        }
    }
    return paths;
}


/**
 * Given pixels and point clouds for all levels
 * Return paths for all levels
 * @param pixels_by_level_and_frame
 * @param point_clouds_by_level_and_frame
 * @return
 */
std::vector<std::vector<std::vector<PixelInFrame>>>
compute_correspondence_paths(const std::vector<std::vector<std::vector<Pixel>>>& pixels_by_level_and_frame,
                             const std::vector<std::vector<Eigen::MatrixX3d>>& point_clouds_by_level_and_frame) {
    using namespace std;

    vector<vector<vector<PixelInFrame>>> paths;
    for( unsigned int level = 0; level < pixels_by_level_and_frame.size(); ++level) {
        cout << "Computing correspondences for level : " << level << endl;
        paths.push_back( compute_correspondence_paths_for_level(pixels_by_level_and_frame.at(level), point_clouds_by_level_and_frame.at(level)));
    }
    return paths;
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
    for( const auto& depth_maps : depth_map_hierarchy) {
        cout << "Extracting valid pixels for level : " << level << endl;
        valid_pixels_for_levels.push_back(get_valid_pixels_for_depth_maps(depth_maps));
        ++level;
    }
    return valid_pixels_for_levels;
}

/**
 * Given a vector (per frame) of valid pixels
 * and a vector (per frame) of cameras
 * and a vector (per frame) of depth maps
 * Return a vector (per frame) of 3D points they represent
 * @param valid_pixels
 * @param cameras
 * @param depth_maps
 * @return
 */
std::vector<Eigen::MatrixX3d>
project_pixels_to_point_clouds(const std::vector<std::vector<Pixel>>& valid_pixels,
                               const std::vector<Camera> &cameras,
                               const std::vector<DepthMap> &depth_maps) {
    using namespace std;

    std::vector<Eigen::MatrixX3d> point_clouds;

    for( unsigned int frame = 0; frame < cameras.size(); ++frame ) {
        point_clouds.push_back(project_pixels_to_point_cloud(valid_pixels.at(frame),
                                                             cameras.at(frame),
                                                             depth_maps.at(frame)));
    }
    return point_clouds;
}

/**
 * Given a vector (per level) or vectors (per frame) of Pixels
 * plus asscoiated depth maps and cameras
 * return a vector (per level) or vectors (per frame) of Eigen X3 matrices conytaining
 * the 3D points.
 * @param valid_pixels_for_levels
 * @param cameras
 * @param depth_maps_per_frame_and_level
 * @return
 */
std::vector<std::vector<Eigen::MatrixX3d>>
project_pixels_to_point_clouds(const std::vector<std::vector<std::vector<Pixel>>>& pixels_per_frame_and_level,
                               const std::vector<Camera> &cameras_per_frame,
                               const std::vector<std::vector<DepthMap>> &depth_maps_per_frame_and_level) {
    using namespace std;

    int num_levels = depth_maps_per_frame_and_level.size();

    std::vector<std::vector<Eigen::MatrixX3d>> point_clouds;
    for( unsigned int level = 0; level < num_levels; ++level) {
        cout << "Computing pointclouds for level : " << level << endl;

        // Clone cameras and adjust for level
        std::vector<Camera> level_cameras;
        for( const auto& camera : cameras_per_frame ) {
            Camera lc{camera.position, camera.view, camera.up, camera.resolution, camera.fov, camera.focalDistance};
            lc.resolution[0] /= (1 << level);
            lc.resolution[1] /= (1 << level);
            level_cameras.push_back(lc);
        }

        point_clouds.push_back( project_pixels_to_point_clouds(pixels_per_frame_and_level.at(level),
                                                               level_cameras,
                                                               depth_maps_per_frame_and_level.at(level)));
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
save_normals_to_file(const std::vector<std::vector<std::vector<Pixel>>>& pixels_for_level_and_frame,
                     const std::vector<std::vector<DepthMap>>& depth_map_hierarchy,
                     const std::string& normal_file_template) {
    for (unsigned int level = 0; level < pixels_for_level_and_frame.size(); ++level) {
        for (unsigned int frame = 0; frame < pixels_for_level_and_frame.at(level).size(); ++frame) {

            save_normals(pixels_for_level_and_frame.at(level).at(frame), depth_map_hierarchy.at(level).at(frame),
                         file_name_from_template_level_and_frame(normal_file_template, level, frame));
        }
    }
}

void save_paths_to_file(const std::vector<std::vector<std::vector<PixelInFrame>>> &paths_by_level,
                        const std::string &path_file_template) {
    using namespace std;

    unsigned int level = 0;
    for (const auto &level_paths : paths_by_level) {
        string file_name = file_name_from_template_and_level(path_file_template, level);
        ofstream file{file_name};
        for (const auto &path : level_paths) {
            for (const auto &pif : path) {
                file << "( " << pif.frame << ", " << pif.pixel.x << ", " << pif.pixel.y << ") ";
            }
            file << endl;
        }
        ++level;
    }
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
    vector<Camera> cameras = load_cameras(num_frames);

    // Construct the hierarchy: number of levels as specified in properties.
    vector<vector<DepthMap>> depth_map_hierarchy = create_depth_map_hierarchy(properties, depth_maps);

    // Compute normals for the hierarchy
    for (auto & depth_maps_for_level : depth_map_hierarchy) {
        for (unsigned int f = 0; f < num_frames; ++f) {
            depth_maps_for_level.at(f).compute_normals(cameras.at(f));
        }
    }

    // Vector of level, frame, pixel index
    vector<vector<vector<Pixel>>> valid_pixels_for_levels = get_valid_pixels_for_all_levels(depth_map_hierarchy);

    // Project valid pixels into point clouds
    vector<vector<Eigen::MatrixX3d>> point_clouds_for_all_levels =
    project_pixels_to_point_clouds(valid_pixels_for_levels, cameras, depth_map_hierarchy);

    // Save point clouds if requested
    if( properties.getBooleanProperty("save-point-clouds")) {
        save_point_clouds_to_file( point_clouds_for_all_levels, properties.getProperty("point-cloud-file-template"));
    }

    // Save normals if requested
    if( properties.getBooleanProperty("save-normals")) {
        save_normals_to_file( valid_pixels_for_levels, depth_map_hierarchy, properties.getProperty("normal-file-template"));
    }

    // Computer Pixel paths across frames
    vector<vector<vector<PixelInFrame>>> corresponding_paths_by_level = compute_correspondence_paths(valid_pixels_for_levels, point_clouds_for_all_levels);

    // Save paths if requested
    if( properties.getBooleanProperty("save-paths")) {
        save_paths_to_file( corresponding_paths_by_level, properties.getProperty("path-file-template"));
    }

    // Cluster and write to level files.
    unsigned int level = 0;
    for( const auto& corrs_for_level : corresponding_paths_by_level) {
        cout << "Saving level " << level << "correspondences" << endl;
        string file_name = file_name_from_template_and_level(properties.getProperty("correspondence-file-template"), level);
        save_correspondences_to_file(file_name, corrs_for_level);
        ++level;
    }
    return 0;
}