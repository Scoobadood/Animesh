//
// Created by Dave Durbin on 11/3/20.
//

#include <vector>
#include <Camera/Camera.h>
#include <DepthMap/Normals.h>
#include <DepthMap/DepthMap.h>

#include <Eigen/SVD>

struct PixelWithDepth {
    const unsigned int x;
    const unsigned int y;
    const float depth;
    PixelWithDepth(unsigned int x, unsigned int y, float depth) : x{x}, y{y}, depth{depth} {};
};

/**
 * Given a depth map and camera plus x,y coordinates
 * find the neighbours of the given point in the depth map
 * with non-zero depth.
 */
std::vector<PixelWithDepth> get_neighbours_for_pixel(const DepthMap *depth_map, int x, int y) {
    using namespace std;

    vector<PixelWithDepth> neighbours;
    float neighbour_depths[8];
    unsigned int available_neighbours = depth_map->get_neighbour_depths(x, y, neighbour_depths, true);
    if (DepthMap::flag_is_set(available_neighbours, DepthMap::UP)) {
        neighbours.emplace_back(x, y - 1, neighbour_depths[0]);
    }
    if (DepthMap::flag_is_set(available_neighbours, DepthMap::DOWN)) {
        neighbours.emplace_back(x, y + 1, neighbour_depths[1]);
    }
    if (DepthMap::flag_is_set(available_neighbours, DepthMap::LEFT)) {
        neighbours.emplace_back(x - 1, y, neighbour_depths[2]);
    }
    if (DepthMap::flag_is_set(available_neighbours, DepthMap::RIGHT)) {
        neighbours.emplace_back(x + 1, y, neighbour_depths[3]);
    }
    if (DepthMap::flag_is_set(available_neighbours, DepthMap::UP_LEFT)) {
        neighbours.emplace_back(x - 1, y - 1, neighbour_depths[4]);
    }
    if (DepthMap::flag_is_set(available_neighbours, DepthMap::UP_RIGHT)) {
        neighbours.emplace_back(x + 1, y - 1, neighbour_depths[5]);
    }
    if (DepthMap::flag_is_set(available_neighbours, DepthMap::DOWN_LEFT)) {
        neighbours.emplace_back(x - 1, y + 1, neighbour_depths[6]);
    }
    if (DepthMap::flag_is_set(available_neighbours, DepthMap::DOWN_RIGHT)) {
        neighbours.emplace_back(x + 1, y + 1, neighbour_depths[7]);
    }
    return neighbours;
}


/**
 * Find the plane that best fits the given set of points.
 * Return the planar coefficients A, B and C and D s.t. Ax + By + Cz + D = 0
 * Return tru if this succeeds
 * Return faslse if
 * There are less than 3 points
 * The points are collinear
 * We fail to compute the plane for some other resson.
 */
bool
fit_plane_to_points(const std::vector<Eigen::Vector3f>& points, Eigen::Vector3f& planar_normal) {
    using namespace Eigen;
    // Reject less than 3 points - can';'t get a plane from these.
    if( points.size() < 3 ) {
        return false;
    }

    // Construct 3xN matrix of points
    Matrix3Xf allPoints{3, points.size()};
    int colIdx = 0;
    for( const auto& point : points ) {
        allPoints.col(colIdx++) = point;
    }

    auto centroid = allPoints.rowwise().mean();
    const Matrix3Xf pointsCentered = allPoints.colwise() - centroid;
    int setting = Eigen::ComputeFullU | Eigen::ComputeThinV;
    BDCSVD<Matrix3Xf> svd = pointsCentered.bdcSvd(setting);
    Vector3f n = svd.matrixU().col( 2);
    planar_normal = n;

    return true;
}

/**
 * Compute normal using estimate of normal to plane tangent to surface
 * using neighbours in depth map
 */
std::vector<std::vector<NormalWithType>>
compute_normals_from_neighbours(DepthMap *depth_map, const Camera &camera) {
    using namespace std;
    using namespace Eigen;

    vector<vector<NormalWithType>> normals;

    // For each pixel
    for (auto y = 0; y < depth_map->height(); ++y) {
        vector<NormalWithType> normals_for_row;
        for (auto x = 0; x < depth_map->width(); ++x) {
            float depth = depth_map->depth_at(x, y);
            if (depth == 0.0f) {
                normals_for_row.emplace_back(NONE, 0.0, 0.0, 0.0);
                continue;
            }

            // Collect neighbours
            vector<PixelWithDepth> neighbours = get_neighbours_for_pixel(depth_map, x, y);

            // Backproject points
            vector<Vector3f> neighbours_in_world_coords;
            neighbours_in_world_coords.reserve(neighbours.size());
            for (const auto &n : neighbours) {
                neighbours_in_world_coords.emplace_back(camera.to_world_coordinates(n.x, n.y, n.depth));
            }

            // Fit Plane
            Vector3f planar_normal;
            if (!fit_plane_to_points(neighbours_in_world_coords, planar_normal)) {
                normals_for_row.emplace_back(NONE, 0.0, 0.0, 0.0);
                continue;
            }

            // Force correct orientation
            Vector3f cam2point = camera.to_world_coordinates(x, y, depth);
            float dp = cam2point.dot(planar_normal);
            if( dp < 0 ) {
                planar_normal = - planar_normal;
            }

            // Store normal to plane
            normals_for_row.emplace_back(NATURAL, planar_normal.x(), planar_normal.y(), planar_normal.z());
        }
        normals.emplace_back(normals_for_row);
    }

    return normals;
}
