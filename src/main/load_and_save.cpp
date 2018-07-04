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
 * Write the field to Matlab
 */
void save_field( const Args& args, Field * field ) {
	write_matlab_file( field, "initial.mat" );

	field->smooth( );
	
	write_matlab_file( field, "final.mat" );
}




/**
 * Construct a field
 */
Field * load_field( const Args& args) {
	Field * field = nullptr;

	bool dump_field = args.should_dump_field();

	if( args.load_from_pointcloud() ) {
		field = load_field_from_obj_file( args.file_name(), args.k(), args.scale(),args.tracing_enabled() );
	} else {
		switch( args.default_shape()	 ) {
			case Args::SPHERE:
				field = Field::spherical_field( args.radius(), args.theta_steps(), args.phi_steps(), args.k() );
				std::cout << "sphere" << std::endl;
				break;

			case Args::CUBE: 
				field = Field::cubic_field( args.cube_x(), args.cube_y(), args.cube_z(), args.scale(), args.k() );
				std::cout << "cube" << std::endl;
				break;

			case Args::CIRCLE: 
				field = Field::circular_field( args.radius(), args.k() );
				std::cout << "circle" << std::endl;
				break;

			case Args::PLANE:
				field = Field::planar_field( args.plane_x(), args.plane_y(), args.grid_spacing(), args.k() );
				std::cout << "planar" << std::endl;
				break;

			case Args::POLYNOMIAL:
				field = Field::polynomial_field( args.plane_x(), args.plane_y(), args.grid_spacing(), args.k());
				std::cout << "polynomial" << std::endl;
				break;
		}
	}

	std::cout << "Built field" << std::endl;
	field->enable_tracing( args.tracing_enabled() );
	return field;
}