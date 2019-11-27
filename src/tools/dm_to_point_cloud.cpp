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
#include <fstream>
#include "../mesher/types.h"
#include "../mesher/correspondences_io.h"
#include "../mesher/utilities.h"

#include <cpd/gauss_transform_fgt.hpp>
//#include <cpd/jsoncpp.hpp>
#include <cpd/rigid.hpp>
#include <iostream>

const char VALID_PIXL_POINT_CLOUD_TEMPLATE[] = "pointcloud_L%02d_F%02d.txt";
const char NORMAL_TEMPLATE[] = "normals_L%02d_F%02d.txt";
const char corr_file_template[] = "corr_L%02d.txt";

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
 * Load the cameras (one per frame)
 */
std::vector<Camera>
load_cameras(unsigned int num_frames) {
    std::vector<Camera> cameras;
    // TODO: Move to loading these from disk rather than constructing by hand.
    cameras.reserve(num_frames);
    for (unsigned int i = 0; i < num_frames; ++i) {
        cameras.emplace_back(
                (float[]) {0.0f, 0.0f, 5.0f}, // position
                (float[]) {0.0f, 0.0f, 0.0f}, // view
                (float[]) {0.0f, 1.0f, 0.0f}, // up
                (float[]) {640.0f, 480.0f},        // resolution
                (float[]) {35.0f, 35.0f},     // fov
                5.0f                 // focal distance
        );
    }
    return cameras;
}

static std::vector<Pixel>
depth_map_to_pixels(const DepthMap &depth_map) {
    using namespace std;

    // Filter out zero depth points
    vector<Pixel> valid_pixels;
    for (unsigned int r = 0; r < depth_map.rows(); ++r) {
        for (unsigned int c = 0; c < depth_map.cols(); ++c) {
            float depth = depth_map.depth_at(r, c);
            if (depth > 0.0f) {
                valid_pixels.emplace_back(c, r);
            }
        }
    }
    return valid_pixels;
}

/*
 * Given a camera, a set of N X,Y coordinates and a depth map, generate camera space 3D coordinates
 * and return as an Nx3 matrix
 */
static Eigen::MatrixX3f
project_pixels_to_pointcloud(const std::vector<Pixel> &pixels,
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
    Eigen::MatrixX3f m{valid_points.size(), 3};
    int row = 0;
    for (const auto &p : valid_points) {
        m(row, 0) = p.x;
        m(row, 1) = p.y;
        m(row, 2) = p.z;
        row++;
    }
    return m;
}

void
save_point_cloud(Eigen::MatrixX3f pointcloud, unsigned int level, unsigned int frame, const char file_name_template[]) {
    using namespace std;

    char file_name[strlen(file_name_template) + 1];
    sprintf(file_name, file_name_template, level, frame);
    ofstream file{file_name};
    for (unsigned int row = 0; row < pointcloud.rows(); ++row) {
        file << pointcloud(row, 0) << ", " << pointcloud(row, 1) << ", " << pointcloud(row, 2) << endl;
    }
}

void save_normals(const std::vector<Pixel>& valid_pixels,
        const DepthMap& depth_map,
        unsigned int level,
        unsigned int frame,
        const char file_name_template[] ) {
    using namespace std;

    char file_name[strlen(file_name_template) + 1];
    sprintf(file_name, file_name_template, level, frame);
    ofstream file{file_name};
    for (const auto& pixel : valid_pixels) {
        auto normal = depth_map.normal_at(pixel.x, pixel.y);
        file << normal.x << ", " << normal.y << ", " << normal.z << endl;
    }
}

void
compute_correspondences(unsigned int level,
                        unsigned int frame,
                        std::map<unsigned int, unsigned int> &correspondence,
                        const std::string& file_name_template) {
    char fixedPoints[file_name_template.length() + 1];
    char movingPoints[file_name_template.length() + 1];
    sprintf(movingPoints, file_name_template.c_str(), level, frame);
    sprintf(fixedPoints, file_name_template.c_str(), level, frame + 1);

    cpd::Matrix fixed = cpd::matrix_from_path(fixedPoints);
    cpd::Matrix moving = cpd::matrix_from_path(movingPoints);
    cpd::Rigid rigid;
    rigid.gauss_transform(std::move(
            std::unique_ptr<cpd::GaussTransform>(new cpd::GaussTransformFgt())));
    rigid.correspondence(true);
    rigid.normalize(false);
    cpd::RigidResult result = rigid.run(fixed, moving);

    correspondence.clear();
    for (unsigned int i = 0; i < result.correspondence.size(); ++i) {
        correspondence.emplace(i, result.correspondence(i));
    }
}

void
merge_groups(std::multimap<unsigned int, PixelInFrame> &group_to_pif,
             std::map<PixelInFrame, unsigned int> &pif_to_group,
             PixelInFrame &pif1,
             unsigned int pif1_group_id,
             PixelInFrame &pif2,
             unsigned int pif2_group_id) {

    using namespace std;

    // Update mapping for pif2 to be in group 1
    pif_to_group.at(pif2) = pif1_group_id;

    // Add mappings for all pif2 group elements to pif1 group
    pair<multimap<unsigned int, PixelInFrame>::iterator, multimap<unsigned int, PixelInFrame>::iterator> ret;
    ret = group_to_pif.equal_range(pif2_group_id);
    for (auto &it = ret.first; it != ret.second; ++it) {
        // Add pif to pif1 group
        group_to_pif.emplace(pif1_group_id, it->second);
    }
    // Remove the pif 2 group
    group_to_pif.erase(pif2_group_id);
}

std::vector<std::vector<Pixel>>
extract_valid_pixels_for_level(const std::vector<DepthMap> &depth_maps) {
    using namespace std;

    int frame_index = 0;
    vector<vector<Pixel>> valid_pixels;
    for (const auto &depth_map : depth_maps) {
        vector<Pixel> valid_pixels_in_frame = depth_map_to_pixels(depth_map);
        valid_pixels.push_back(valid_pixels_in_frame);

        frame_index++;
    }
    return valid_pixels;
}

std::vector<std::vector<std::vector<Pixel>>>
extract_valid_pixels_for_all_levels(const std::vector<std::vector<DepthMap>> &depth_map_hierarchy) {
    using namespace std;

    int num_levels = depth_map_hierarchy.size();
    int level = num_levels - 1;
    vector<vector<vector<Pixel>>> valid_pixels_for_levels;

    for( unsigned int level = 0; level < num_levels; ++level) {
        cout << "Extracting valid pixels for level : " << (level + 1)<< endl;
        valid_pixels_for_levels.push_back(extract_valid_pixels_for_level(depth_map_hierarchy.at(level)));
    }
    return valid_pixels_for_levels;
}

std::vector<Eigen::MatrixX3f>
project_pixels_to_point_clouds(const std::vector<std::vector<Pixel>>& valid_pixels,
                               const std::vector<Camera> &cameras,
                               const std::vector<DepthMap> &depth_maps) {
    using namespace std;

    std::vector<Eigen::MatrixX3f> point_clouds;

    for( unsigned int frame = 0; frame < cameras.size(); ++frame ) {
        point_clouds.push_back(project_pixels_to_pointcloud(valid_pixels.at(frame),
                                                            cameras.at(frame),
                                                            depth_maps.at(frame)));
    }
    return point_clouds;
}

std::vector<std::vector<Eigen::MatrixX3f>>
project_pixels_to_point_clouds(const std::vector<std::vector<std::vector<Pixel>>>& valid_pixels_for_levels,
                               const std::vector<Camera> &cameras,
                               const std::vector<std::vector<DepthMap>> &depth_map_hierarchy) {
    using namespace std;

    int num_levels = depth_map_hierarchy.size();
    int level = num_levels - 1;


    std::vector<std::vector<Eigen::MatrixX3f>> point_clouds;
    for( unsigned int level = 0; level < num_levels; ++level) {
        cout << "Computing pointclouds for level : " << level << endl;

        // Clone cameras
        std::vector<Camera> level_cameras;
        for( const auto& camera : cameras ) {
            Camera lc{camera.position, camera.view, camera.up, camera.resolution, camera.fov, camera.focalDistance};
            lc.resolution[0] /= (1 << level);
            lc.resolution[1] /= (1 << level);

            level_cameras.push_back(lc);
        }

        // Transform cameras for this level by adjusting their screen resolution
        point_clouds.push_back( project_pixels_to_point_clouds( valid_pixels_for_levels.at(level),
                level_cameras,
                depth_map_hierarchy.at(level)));
    }
    return point_clouds;
}


void
update_correspondence_groups(
        const std::map<unsigned int, unsigned int> &corr,
        const std::vector<std::vector<Pixel>> &valid_pixels_for_level,
        unsigned int from_frame,
        std::multimap<unsigned int, PixelInFrame> &group_to_pif,
        std::map<PixelInFrame, unsigned int> &pif_to_group
) {
    using namespace std;

    // Use corr to update the correspondence groups
    for (const auto &item : corr) {

        PixelInFrame pif1{valid_pixels_for_level.at(from_frame).at(item.first), from_frame};
        PixelInFrame pif2{valid_pixels_for_level.at(from_frame + 1).at(item.second), from_frame + 1};

        // Lookup groups for first and second pifs
        auto it = pif_to_group.find(pif1);
        unsigned int pif1_group_id = (it == pif_to_group.end()) ? 0 : it->second;
        it = pif_to_group.find(pif2);
        unsigned int pif2_group_id = (it == pif_to_group.end()) ? 0 : it->second;

        // If first has a group but not second, add second to first's group
        if (pif1_group_id != 0 && pif2_group_id == 0) {
            // Add second pif to first group
            group_to_pif.emplace(pif1_group_id, pif2);
            pif_to_group.emplace(pif2, pif1_group_id);
        }
            // If second has a group but not first, add first to second's group
        else if (pif1_group_id == 0 && pif2_group_id != 0) {
            // Add first pif to second group
            group_to_pif.emplace(pif2_group_id, pif1);
            pif_to_group.emplace(pif1, pif2_group_id);
        }
            // If neither has a group, create a new one and add both
        else if (pif1_group_id == 0 && pif2_group_id == 0) {
            unsigned int next_group_id = group_to_pif.size() + 1;
            // Add first and second pifs to a new group
            pif_to_group.emplace(pif1, next_group_id);
            pif_to_group.emplace(pif2, next_group_id);
            group_to_pif.emplace(next_group_id, pif1);
            group_to_pif.emplace(next_group_id, pif2);
            ++next_group_id;
        }
            // Both are in the groups but the groups are different
        else if (pif1_group_id != pif2_group_id) {
            merge_groups(group_to_pif, pif_to_group, pif1, pif1_group_id, pif2, pif2_group_id);
        }
        // By default they are both in the same group and no action is required.
    }
}

std::multimap<unsigned int, PixelInFrame>
compute_correspondences_for_level(
        unsigned int level,
        const std::vector<std::vector<Pixel>> &valid_pixels_for_level,
        const std::string& pointcloud_file_name_template) {
    using namespace std;
    using namespace std::chrono;

    int num_frames = valid_pixels_for_level.size();

    cout << "Finding correspondences for level : " << level << endl;
    multimap<unsigned int, PixelInFrame> group_to_pif;
    map<PixelInFrame, unsigned int> pif_to_group;

    for (unsigned int from_frame = 0; from_frame < num_frames - 1; ++from_frame) {
        //std::chrono
        milliseconds start = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
        cout << "  From frame: " << from_frame << " to frame: " << (from_frame + 1) << endl;
        // Gen correspondences
        std::map<unsigned int, unsigned int> corr;
        compute_correspondences(level, from_frame, corr, pointcloud_file_name_template);

        milliseconds duration = duration_cast<milliseconds>(system_clock::now().time_since_epoch()) - start;
        float elapsed = (duration.count() - (duration.count() % 100)) / 1000.0f;

        update_correspondence_groups(
                corr,
                valid_pixels_for_level,
                from_frame,
                group_to_pif,
                pif_to_group);

        cout << "  done in: " << elapsed << "s" << endl;
    }
    return group_to_pif;
}

void
write_correspondences_to_file(
        unsigned int level,
        const std::multimap<unsigned int, PixelInFrame> &group_to_pif) {
    using namespace std;

    // Write correspondences
    char corr_file_name[strlen(corr_file_template) + 1];
    sprintf(corr_file_name, corr_file_template, level);
    ofstream corr_file{corr_file_name};
    for (const auto &group : group_to_pif) {
        corr_file << group.first << "--> (" << group.second.frame << ":" << group.second.pixel.x << ","
                  << group.second.pixel.y << ")" << endl;
    }
}

std::vector<std::multimap<unsigned int, PixelInFrame>>
compute_correspondences(
        const std::vector<std::vector<std::vector<Pixel>>> &valid_pixels_for_levels,
        const std::string& pointcloud_file_name_template) {
    using namespace std;

    vector<multimap<unsigned int, PixelInFrame>> corrs_by_level;

    // Returning results which we _also_ write to file.
    for( unsigned int level = 0; level < valid_pixels_for_levels.size(); ++level) {
        multimap<unsigned int, PixelInFrame> group_to_pif = compute_correspondences_for_level(
                level,
                valid_pixels_for_levels.at(level),
                pointcloud_file_name_template);
        corrs_by_level.push_back(group_to_pif);
    }
    return corrs_by_level;
}

void
write_correspondences(const std::vector<std::multimap<unsigned int, PixelInFrame>>& correspondences ) {
    unsigned int level = 0;
    for( const auto& level_corrs : correspondences ) {
        write_correspondences_to_file(level, level_corrs);
        ++level;
    }
}

void compute_and_save_correspondences(
        const std::vector<std::vector<std::vector<Pixel>>> &valid_pixels_for_levels,
        const std::string& pointcloud_file_name_template) {
    using namespace std;

    // Returning results which we _also_ write to file.
    int level = valid_pixels_for_levels.size() - 1;
    while (level >= 0) {
        multimap<unsigned int, PixelInFrame> group_to_pif = compute_correspondences_for_level(
                level,
                valid_pixels_for_levels.at(level),
                pointcloud_file_name_template);
        write_correspondences_to_file(level, group_to_pif);
        --level;
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

    // Compute normals
    for (unsigned int l = 0; l < depth_map_hierarchy.size(); ++l) {
        for (unsigned int f = 0; f < num_frames; ++f) {
            depth_map_hierarchy.at(l).at(f).compute_normals(cameras.at(f));
        }
    }

    // Vector of level, frame, pixel index
    vector<vector<vector<Pixel>>> valid_pixels_for_levels = extract_valid_pixels_for_all_levels(depth_map_hierarchy);

    // Project valid pixels into point clouds
    vector<vector<Eigen::MatrixX3f>> point_clouds_for_all_levels =
    project_pixels_to_point_clouds(valid_pixels_for_levels, cameras, depth_map_hierarchy);
    for (unsigned int level = 0; level < point_clouds_for_all_levels.size(); ++level) {
        for (unsigned int frame = 0; frame < point_clouds_for_all_levels.at(level).size(); ++frame) {
            save_point_cloud(point_clouds_for_all_levels.at(level).at(frame), level, frame,
                             VALID_PIXL_POINT_CLOUD_TEMPLATE);
        }
    }

    // Save also the normals for each valid pixel
    for (unsigned int level = 0; level < valid_pixels_for_levels.size(); ++level) {
        for (unsigned int frame = 0; frame < valid_pixels_for_levels.at(level).size(); ++frame) {
            save_normals(valid_pixels_for_levels.at(level).at(frame), depth_map_hierarchy.at(level).at(frame), level, frame,
                             NORMAL_TEMPLATE);
        }
    }

#ifndef FUDGE_FLAG
    // Now compute correspondences
    vector<multimap<unsigned int, PixelInFrame>> corrs_by_level = compute_correspondences(valid_pixels_for_levels, VALID_PIXL_POINT_CLOUD_TEMPLATE);
    write_correspondences(corrs_by_level);

#else
    // TEMPORARY FUDGE TO READ CORRS FROM FILE RATHER THAN GENERATE THEM


        // END FUDGE
#endif
    // Cluster and write to level files.
    unsigned int level = 0;
    for( const auto& corrs_for_level : corrs_by_level) {
        // Convert multimap of pif to vec<vec<pif>>
        vector<vector<PixelInFrame>> out;
        for (auto it1 = corrs_for_level.begin(), it2 = it1, end = corrs_for_level.end(); it1 != end; it1 = it2){
            out.emplace_back();
            for ( ; it1->first == it2->first; ++it2){
                out.back().push_back(it2->second);
            }
        }


        cout << "Saving level " << level << "correspondences" << endl;
        char file_name[1024];
        sprintf(file_name, "level_%02d_corr.bin", level);
        save_correspondences_to_file(file_name, out);
        ++level;
    }
    return 0;
}