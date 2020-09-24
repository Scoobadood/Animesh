//
// Compute normals using PCL
//

#include "DepthMap.h"
#include "Normals.h"
#include <Camera/Camera.h>

#include <pcl/point_types.h>
#include <pcl/features/normal_3d.h>


/**
 * Compute normal using estinate of normal to plane tangent to surface
 */
std::vector<std::vector<NormalWithType>>
compute_normals_with_pcl(DepthMap* depth_map, const Camera& camera) {
    using namespace pcl;
    using namespace std;

    struct Pixel {
        unsigned int x;
        unsigned int y;
        Pixel(unsigned int x, unsigned int y) : x{x}, y{y}{
            // Empty
        };
        bool operator< (const Pixel &other) const {
            if( y != other.y)
                return y < other.y;

            return x < other.x;
        }
    };

    // Count number of legitimate normals
    vector<Pixel> valid_pixels;
    unsigned int num_points = 0;
    for (int y = 0; y < depth_map->height(); ++y) {
        for (int x = 0; x < depth_map->width(); ++x) {
            if( depth_map->depth_at(x, y) != 0.0f) {
                num_points++;
                valid_pixels.emplace_back(x,y);
            }
        }
    }

    PointCloud<PointXYZ>::Ptr cloud(new pcl::PointCloud <pcl::PointXYZ>);
    cloud->points.resize( num_points );

    // Populate the point cloud
    int i = 0;
    for (const auto& pixel : valid_pixels) {
        float d = depth_map->depth_at(pixel.x, pixel.y);
        // Backproject the depths to get world coord
        Eigen::Vector3f a = camera.to_world_coordinates(pixel.x, pixel.y, d);
        cloud->points[i].getVector3fMap() = a;
        ++i;
    }

    // Create the normal estimation class, and pass the input dataset to it
    NormalEstimation <PointXYZ, Normal> ne;
    ne.setInputCloud(cloud);

    // Create an empty kdtree representation, and pass it to the normal estimation object.
    // Its content will be filled inside the object, based on the given input dataset (as no other search surface is given).
    search::KdTree<PointXYZ>::Ptr tree(new search::KdTree<PointXYZ>());
    ne.setSearchMethod(tree);

    // Output datasets
    PointCloud<Normal>::Ptr cloud_normals(new pcl::PointCloud <pcl::Normal>);

    // Use all neighbors in a sphere of radius 3cm
//    ne.setRadiusSearch(0.03);
    // Use nearest 5 neighbours regardless
    ne.setKSearch(15);

    // Set the view point to disambiguate normal direction
    ne.setViewPoint(camera.origin().x(), camera.origin().y(), camera.origin().z() );

    // Compute the features
    ne.compute(*cloud_normals);

    // cloud_normals->points.size () should have the same size as the input cloud->points.size ()*


    // Now populate the normal_types and normals
    vector<vector<NormalWithType>> normals{depth_map->height() * depth_map->width()};

    for( int y=0; y<depth_map->height(); ++y ) {
        vector<NormalWithType> normal_row;
        for( int x=0; x<depth_map->width(); ++x ) {
            normal_row.emplace_back(NONE, 0, 0, 0);
        }
        normals.push_back(normal_row);
    }

    // Go back and put in the actual details
    for( const auto& pixel : valid_pixels ) {
        normals.at(pixel.y).at(pixel.x).type = NATURAL;
        normals.at(pixel.y).at(pixel.x).x = cloud_normals->points[i].normal_x;
        normals.at(pixel.y).at(pixel.x).y = cloud_normals->points[i].normal_y;
        normals.at(pixel.y).at(pixel.x).z = cloud_normals->points[i].normal_z;
    }

    return normals;
}