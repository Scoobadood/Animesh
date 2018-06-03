#include <fstream>
#include <sstream>
#include <stdlib.h>
#include <iostream>

#include "load_and_save.h"

#include <Field/FieldExporter.h>
#include <Field/MatlabFieldExporter.h>


const int EXPORT_FRAMES = 10;
const std::string OUTPUT_DIRECTORY = "/Users/dave/Desktop/animesh_output";
const std::string OUTPUT_FILE_ROOT = "frame";


void write_matlab_file( Field * field, const std::string& file_name ) {
	std::ostringstream oss;
	oss << OUTPUT_DIRECTORY << "/" << file_name;
	std::ofstream file{ oss.str() };
	FieldExporter * fe = new MatlabFieldExporter( file );
	fe->exportField( *field );
	delete fe;
}

void write_matlab_file( Field * field, int index ) {
	std::ostringstream oss;
	oss << OUTPUT_FILE_ROOT << index << ".mat";
	write_matlab_file( field, oss.str());
}


/**
 * Load an obj file into a point cloud
 */
pcl::PointCloud<pcl::PointXYZ>::Ptr load_pointcloud_from_obj( const std::string& file_name ) {
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
 * Construct a field from an OBJ file
 */
Field * load_field_from_obj_file( const Args& args ) {
	std::string file_name = args.pcd_file_name();
	int k = args.k();

	// Load the point cloud from file
	pcl::PointCloud<pcl::PointXYZ>::Ptr cloud = load_pointcloud_from_obj(file_name);
	if( !cloud ) 
		return nullptr;

	// Now construct a graph
    // Iterate through all points and find K nearest neighbours. Create edges
    Graph * graph = new Graph();

    // For each point in the cloud, add a FieldElement node to the graph
    for( auto it = cloud->begin(); it != cloud->end(); ++it ) {
    	pcl::PointXYZ point = *it;
    	FieldElement * fe = new FieldElement( 
    		Eigen::Vector3f{ point[0], point[1], point[2]},
    		Eigen::Vector3f{ 0.0f, 0.0f, 0.0f},
    		Eigen::Vector3f::Zero);

        graph->add_node( fe );
    }

    // Make a KD-tree for the point cloud
    pcl::KdTreeFLANN<pcl::PointXYZ> kdtree;
    kdtree.setInputCloud (cloud);

    // Vectors to store found points
    std::vector<int> pointIdxNKNSearch(k);
    std::vector<float> pointNKNSquaredDistance(k);
    float weight = 1.0f;

    // Now iterate over all field elements and find K neighbours
    for( auto it = graph->begin(); it != graph->end(); ++it ) {
    	FieldElement * fe = *it;
        pcl::PointXYZ point{ fe->m_location[0], fe->m_location[1], fe->m_location[2] };
        // Search
        if ( kdtree.nearestKSearch (point, k, pointIdxNKNSearch, pointNKNSquaredDistance) > 0 ) {
            for (size_t i = 0; i < pointIdxNKNSearch.size (); ++i)
                graph->add_edge( point, &(cloud->points[ pointIdxNKNSearch[i] ]), weight, nullptr );
        }
    }

    // Graph is built
    std::cout << "Done" << std::endl;
}



/**
 * Construct a field
 */
Field * load_field( const Args& args) {
	Field * field = nullptr;

	bool make_field_fixed = args.should_fix_tangents();
	bool dump_field = args.should_dump_field();

	if( args.load_from_pointcloud() ) {
		field = load_field_from_obj_file( args );
	} else {
		switch( args.default_shape()	 ) {
			case Args::SPHERE:
				field = Field::spherical_field( args.radius(), args.theta_steps(), args.phi_steps(), args.k(), make_field_fixed );
				std::cout << "sphere" << std::endl;
				break;

			case Args::CUBE: 
				field = Field::cubic_field( args.cube_size(), make_field_fixed );
				std::cout << "cube" << std::endl;
				break;

			case Args::CIRCLE: 
				field = Field::circular_field( args.radius(), args.k(), make_field_fixed );
				std::cout << "circle" << std::endl;
				break;

			case Args::PLANE:
				field = Field::planar_field( args.plane_x(), args.plane_y(), args.grid_spacing(), make_field_fixed );
				std::cout << "planar" << std::endl;
				break;

			case Args::POLYNOMIAL:
				field = Field::polynomial_field( args.plane_x(), args.plane_y(), args.grid_spacing(), make_field_fixed );
				std::cout << "polynomial" << std::endl;
				break;
		}
	}

	std::cout << "Built field" << std::endl;
	field->enable_tracing( args.tracing_enabled() );
	return field;
}

/**
 * Write the field to Matlab
 */
void save_field( const Args& args, Field * field ) {
	write_matlab_file( field, "initial.mat" );

	int frame_index = 0;
	for( int i=0; i< args.num_iterations(); ++i ) {
		field->smooth_once( );
		float err = field->error();
		std::cout << err << std::endl;
		if( err < 1 ) break;

		if( i % EXPORT_FRAMES == 0 )
			write_matlab_file( field, frame_index++ );
	}
	
	write_matlab_file( field, "final.mat" );
}
