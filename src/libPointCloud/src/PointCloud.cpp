#include <PointCloud/PointCloud.h>
#include <pcl/io/pcd_io.h>

PointCloud * PointCloud::load_from_file( const std::string& file_name ) {
    pcl::PointCloud<pcl::PointXYZ>::Ptr cloud (new pcl::PointCloud<pcl::PointXYZ>);

    if ( pcl::io::loadPCDFile<pcl::PointXYZ> (file_name, *cloud) == -1) {
        PCL_ERROR ("Couldn't read file test_pcd.pcd \n");
        return nullptr;
    }

    std::cout << "Loaded "
            << cloud->width * cloud->height
            << " data points from test_pcd.pcd with the following fields: "
            << std::endl;
    for (size_t i = 0; i < cloud->points.size (); ++i)
    std::cout << "    " << cloud->points[i].x
              << " "    << cloud->points[i].y
              << " "    << cloud->points[i].z << std::endl;

    PointCloud * pc = new PointCloud( cloud );
    pc->compute_normals();

    return pc;
}

/**
 * @return the size of the point cloud (number of points).
 */
size_t PointCloud::size( ) const {
    return points->size();
}

/**
 * Add a point
 */
void PointCloud::add_point( Eigen::Vector3f point ) {
    pcl::PointXYZ p;
    p.x = point[0]; p.y = point[1]; p.z = point[2];
    points->push_back( p );
}

/**
 * Subscript operator
 */
const Point PointCloud::point( size_t index ) const {
    return Point{
        Eigen::Vector3f{ points->points[index].x * 100, points->points[index].y * 100, points->points[index].z * 100},
        Eigen::Vector3f{ normals->points[index].normal_x, normals->points[index].normal_y, normals->points[index].normal_z},
    };
}

/**
 * Compute the normals for this point cloud
 */
void PointCloud::compute_normals( ) {
    using namespace pcl;

    // Build normals
    // Create the normal estimation class, and pass the input dataset to it
    NormalEstimation<PointXYZ, Normal> ne;
    ne.setInputCloud (points);

    // Create an empty kdtree representation, and pass it to the normal estimation object.
    // Its content will be filled inside the object, based on the given input dataset (as no other search surface is given).
    search::KdTree<PointXYZ>::Ptr tree (new search::KdTree<PointXYZ> ());
    ne.setSearchMethod (tree);
    ne.setViewPoint( 0.0f, 0.0f, 10.0f );

    // Use 10 neghbours
    ne.setKSearch (10);

    // Compute the features
    ne.compute (*normals);
}


