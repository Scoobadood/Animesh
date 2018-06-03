#include <pcl/io/obj_io.h>
#include <pcl/kdtree/kdtree_flann.h>

#include <Args/Args.h>
#include <FileUtils/FileUtils.h>
#include <Graph/Graph.h>


pcl::PointCloud<pcl::PointXYZ>::Ptr load_from_file( const std::string& file_name ) {
    using namespace std;
    using namespace pcl;

    if( file_name.size() == 0 ) 
        throw std::invalid_argument( "Missing file name" );

    bool is_directory;
    if (!file_exists(file_name, is_directory ) )
        throw std::runtime_error( "File not found: " + file_name );

    pcl::PointCloud<pcl::PointXYZ>::Ptr cloud (new pcl::PointCloud<pcl::PointXYZ>);
    if( pcl::io::loadOBJFile(file_name, *cloud) == -1) {
        PCL_ERROR ("Couldn't read file\n");
        return nullptr;
    }

    return cloud;
}


/**
 * Main entry point
 */
int main( int argc, char * argv[] ) {
	Args args{ argc, argv};

	// Load the point cloud from file
	pcl::PointCloud<pcl::PointXYZ>::Ptr cloud = load_from_file( args.pcd_file_name());

	// Now construct a graph
    // Iterate through all points and find K nearest neighbours. Create edges
    Graph * graph = new Graph();

    // For each point, add  node to the graph
    for( auto it = cloud->begin(); it != cloud->end(); ++it ) {
        graph->add_node( &(*it) );
    }

    // Make a KD-tree for the point cloud
    pcl::KdTreeFLANN<pcl::PointXYZ> kdtree;
    kdtree.setInputCloud (cloud);

    // Vectors to store found points
    int k = 10;
    std::vector<int> pointIdxNKNSearch(k);
    std::vector<float> pointNKNSquaredDistance(k);
    float weight = 1.0f;

    // Now iterate over all points and find K neighbours
    for( auto it = cloud->begin(); it != cloud->end(); ++it ) {
        pcl::PointXYZ * point = &(*it);
        // Search
        if ( kdtree.nearestKSearch (*point, k, pointIdxNKNSearch, pointNKNSquaredDistance) > 0 ) {
            for (size_t i = 0; i < pointIdxNKNSearch.size (); ++i)
                graph->add_edge( point, &(cloud->points[ pointIdxNKNSearch[i] ]), weight, nullptr );
        }
    }

    // Graph is built
    std::cout << "Done" << std::endl;
}
