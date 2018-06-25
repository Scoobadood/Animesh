#include <fstream>
#include <sstream>
#include <stdlib.h>
#include <iostream>

#include <pcl/io/obj_io.h>

#include "load_and_save.h"

#include <FileUtils/FileUtils.h>
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
pcl::PointCloud<pcl::PointNormal>::Ptr load_pointcloud_from_obj( const std::string& file_name ) {
    if( file_name.size() == 0 ) 
        throw std::invalid_argument( "Missing file name" );

    bool is_directory;
    if (!file_exists(file_name, is_directory ) )
        throw std::runtime_error( "File not found: " + file_name );

    pcl::PointCloud<pcl::PointNormal>::Ptr cloud (new pcl::PointCloud<pcl::PointNormal>);
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
	std::string file_name = args.file_name();
	std::cout << "Loading from file " << file_name << std::endl;

	// Load the point cloud from file
	pcl::PointCloud<pcl::PointNormal>::Ptr cloud = load_pointcloud_from_obj(file_name);
	if( !cloud ) 
		return nullptr;

	// Scale points
	std::cout << "scaling points by " << args.scale() << std::endl;
	float scale = args.scale();
	for( auto iter = cloud->points.begin(); iter != cloud->points.end(); ++iter ) {
		(*iter).x *= scale;
		(*iter).y *= scale;
		(*iter).z *= scale;
	}

	std::cout << "building graph with " << args.k() << " nearest neighbours." << std::endl;
	return new Field( cloud, args.k(), args.tracing_enabled() );
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
				field = Field::cubic_field( args.cube_size(), args.scale(), make_field_fixed );
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

	field->smooth( );
	
	write_matlab_file( field, "final.mat" );
}
