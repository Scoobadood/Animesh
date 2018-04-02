#include <fstream>
#include <sstream>
#include <stdlib.h>
#include <iostream>

#include <Graph/Graph.h>
#include <Graph/GridGraphBuilder.h>
#include <Graph/NearestNeighbourGraphBuilder.h>
#include <Element/Element.h>
#include <Field/Field.h>
#include <Field/FieldExporter.h>
#include <Field/FieldBuilder.h>
#include <Field/MatlabFieldExporter.h>
#include <PointCloud/PointCloud.h>
#include <Args/Args.h>
#include <vector>

using namespace Eigen;

const int TRI_RADIUS = 4;
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

int main( int argc, char * argv[] ) {
	Args args{ argc, argv};

	bool make_field_fixed = args.should_fix_tangents();
	bool dump_field = args.should_dump_field();

	Field * field = nullptr;

	// switch( args.default_shape()	 ) {
	// 	case Args::SPHERE:
	// 		field = Field::spherical_field( args.radius(), args.theta_steps(), args.phi_steps(), make_field_fixed );
	// 		std::cout << "sphere" << std::endl;
	// 		break;

	// 	case Args::CUBE: 
	// 		field = Field::cubic_field( args.cube_size(), make_field_fixed );
	// 		std::cout << "cube" << std::endl;
	// 		break;

	// 	case Args::PLANE:
	// 		field = Field::planar_field( args.plane_x(), args.plane_y(), args.grid_spacing(), make_field_fixed );
	// 		std::cout << "planar" << std::endl;
	// 		break;
	// }

	PointCloud * pcl = PointCloud::load_from_file( "../data/bunny.pcd");
	field = new Field( pcl );

	field->enable_tracing( args.tracing_enabled() );


	// PointCloud * pc = PointCloud::load_from_file( "" );
	// Field * field = build_field_from_point_cloud( pc );
	// delete pc;

	write_matlab_file( field, "initial.mat" );

	int frame_index = 0;
	for( int i=0; i< args.num_iterations(); ++i ) {
		field->smooth_once( );
		float smoo = field->smoothness();
		std::cout << smoo << std::endl;
		if( smoo < 1 ) break;

		if( i % EXPORT_FRAMES == 0 )
			write_matlab_file( field, frame_index++ );
	}
	
	write_matlab_file( field, "final.mat" );

	delete field;
    return 0;
}
