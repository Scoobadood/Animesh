//
// Created by Dave Durbin on 7/3/20.
//

#include "DepthMap.h"
#include "Normals.h"
#include <Camera/Camera.h>
#include <vector>
#include <Eigen/Geometry>

/**
 * Compute normals that have all adjacent data set in the depth map,
 * that is, those normals which can be computed fully from depth data alone.
 * // TODO: An alternative approach is here: https://cromwell-intl.com/3d/normals.html
 * // Currently using this: https://stackoverflow.com/questions/34644101/calculate-surface-normals-from-depth-image-using-neighboring-pixels-cross-produc
 *
 *
 */
std::vector<std::vector<NormalWithType>>
compute_natural_normals(const DepthMap * const depth_map, const Camera &camera) {
    using namespace std;

    vector<vector<NormalWithType>> normals{depth_map->height() * depth_map->width()};

    // For each row
    for (int y = 0; y < depth_map->height(); ++y) {

        // Temp storage for the y's type and normals
        vector<NormalWithType> current_row_normals;

        // For each column
        for (int x = 0; x < depth_map->width(); ++x) {
            float d = depth_map->depth_at(x, y);

            // If depth is 0 then there's no normal to be had here.
            if (d == 0.0f) {
                current_row_normals.emplace_back(NONE, 0, 0, 0);
                continue;
            }

            float neighbour_depths[4];
            int neighbours_present = depth_map->get_neighbour_depths(x, y, neighbour_depths, false);

            // If there are not four neighbours then I have a derived normal
            if (neighbours_present != DepthMap::FOUR) {
                current_row_normals.emplace_back(DERIVED, 0, 0, 0);
                continue;
            }

            // Backproject the depths
            // TODO: This is super slow and we sill be calculating the same points over and over
            // We should avoid this if possible by back projecting all points once

            // Otherwise I have a natural normal
            Eigen::Vector3f a = camera.to_world_coordinates(x + 1, y, neighbour_depths[3]);
            Eigen::Vector3f b = camera.to_world_coordinates(x - 1, y, neighbour_depths[2]);
            Eigen::Vector3f c1 = a - b;
            a = camera.to_world_coordinates(x, y + 1, neighbour_depths[1]);
            b = camera.to_world_coordinates(x, y - 1, neighbour_depths[0]);
            Eigen::Vector3f c2 = a - b;
            auto n = c1.cross(c2);
            n.normalize();
            current_row_normals.emplace_back(NATURAL, n.x(), n.y(), n.z());
        }
        normals.push_back(current_row_normals);
    }
    return normals;
}

void
compute_derived_normals(const DepthMap * const depth_map, std::vector<std::vector<NormalWithType>>& normals) {
    using namespace std;

    for (size_t row = 0; row < depth_map->height(); ++row) {
        for (size_t col = 0; col < depth_map->width(); ++col) {
            // Skip existing normals
            if (normals.at(row).at(col).type == NONE
                || normals.at(row).at(col).type == NATURAL) {
                continue;
            }

            vector<float> sum{0.0f, 0.0f, 0.0f};
            int count = 0;
            for (int ri = (int)(row - 1); ri <= row + 1; ri++) {
                for (int ci = (int)(col - 1); ci <= col + 1; ci++) {
                    if (ri < 0 || ri >= depth_map->width() || ci < 0 || ci >= depth_map->width()) {
                        continue;
                    }
                    if (normals.at(ri).at(ci).type == NONE) {
                        continue;
                    }
                    if (normals.at(ri).at(ci).type == NATURAL) {
                        float nx = normals.at(ri).at(ci).x;
                        float ny = normals.at(ri).at(ci).y;
                        float nz = normals.at(ri).at(ci).z;
                        sum[0] += nx;
                        sum[1] += ny;
                        sum[2] += nz;
                        count++;
                    }
                }
            }
            // If count == 0; kill this one
            if (count == 0) {
                normals.at(row).at(col).type = NONE;
            } else {
                float mean_nx = sum[0] / count;
                float mean_ny = sum[1] / count;
                float mean_nz = sum[2] / count;
                float l = sqrt(mean_nx * mean_nx + mean_ny * mean_ny + mean_nz * mean_nz);
                normals.at(row).at(col).x = mean_nx / l;
                normals.at(row).at(col).y = mean_ny / l;
                normals.at(row).at(col).z = mean_nz / l;
            }
        }
    }
}
