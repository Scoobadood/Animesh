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

#include "vtkActor.h"
#include "vtkRenderer.h"
#include "vtkCylinderSource.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkPolyDataMapper.h"
#include "vtkFloatArray.h"
#include "vtkPointData.h"
#include "vtkCellArray.h"
#include "vtkProperty.h"
#include "vtkLine.h"

// For sleep
#include <chrono> // std::chrono::microseconds
#include <thread> // std::this_thread::sleep_for;

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

Field * load_field( const Args& args) {
	Field * field = nullptr;

	bool make_field_fixed = args.should_fix_tangents();
	bool dump_field = args.should_dump_field();

	if( args.load_from_pointcloud() ) {
		PointCloud * pcl = PointCloud::load_from_file( args.pcd_file_name() );
		field = new Field( pcl, args.k() );
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


	field->enable_tracing( args.tracing_enabled() );
	return field;
}

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

/**
 * Convert the field to a set of points for rendering
 */
vtkSmartPointer<vtkPolyData> field_to_poly( const Field * const field ) {
	using namespace Eigen;

	vtkSmartPointer<vtkPoints>     	pts  = vtkSmartPointer<vtkPoints>::New();
    vtkSmartPointer<vtkPolyData>	poly = vtkSmartPointer<vtkPolyData>::New();
    vtkSmartPointer<vtkCellArray>	verts = vtkSmartPointer<vtkCellArray>::New();
    vtkSmartPointer<vtkCellArray>	lines = vtkSmartPointer<vtkCellArray>::New();

    const std::vector<const FieldElement *> elements = field->elements();
    for (auto it = elements.begin(); it != elements.end(); ++it ) {
    	const FieldElement * const fe = *it;

    	Vector3f location = fe->m_location;
    	Vector3f tangent = fe->m_tangent;
    	Vector3f normal = fe->m_normal;

    	// Add the field node
    	vtkIdType pid[5];

    	// Add location
  		pid[0] = pts->InsertNextPoint(location.x(), location.y(), location.z() );

        // Add tangent points
        Vector3f p1 = location + tangent;
        pid[1] = pts->InsertNextPoint(p1.x(), p1.y(), p1.z());

        // Add opposite tangent points
        Vector3f p2 = location - tangent;
        pid[2] = pts->InsertNextPoint(p2.x(), p2.y(), p2.z());

        // Compute 90 tangent
        Vector3f ninety = tangent.cross( normal );
        Vector3f p3 = location + ninety;
        pid[3] = pts->InsertNextPoint(p3.x(), p3.y(), p3.z());
        Vector3f p4 = location - ninety;
        pid[4] = pts->InsertNextPoint(p4.x(), p4.y(), p4.z());

		verts->InsertNextCell(5, pid);

    	vtkSmartPointer<vtkLine> line = vtkSmartPointer<vtkLine>::New();
		for( int i=0; i<4; i++ ) {
			line->GetPointIds()->SetId(0,pid[0]);
			line->GetPointIds()->SetId(1,pid[i + 1]);
			lines->InsertNextCell(line);
		}

        // // Add opposite tangent points
        // pts->InsertNextPoint(
        // 	fe->m_location[0] - fe->m_normal[0],
        // 	fe->m_location[1] - fe->m_normal[1], 
        // 	fe->m_location[2] - fe->m_normal[2]);
    }
    poly->SetPoints(pts);
    poly->SetVerts(verts);
	poly->SetLines(lines);
    
	return poly;
}

void render_field( const Field * const field ) {
	std::cout << "Getting polydata" << std::endl;
	vtkSmartPointer<vtkPolyData> polydata = field_to_poly( field );

	std::cout << "Creating Mapper" << std::endl;
	vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	mapper->SetInputData(polydata);

	std::cout << "Creating Actor" << std::endl;
	vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
  	actor->SetMapper(mapper);
  	actor->GetProperty()->SetPointSize(3); 

  	vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
	renderer->AddActor(actor);

  	vtkSmartPointer<vtkRenderWindow> renderWindow = vtkSmartPointer<vtkRenderWindow>::New();
  	renderWindow->AddRenderer(renderer);

  	vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
  	renderWindowInteractor->SetRenderWindow(renderWindow);
 
	renderWindow->Render();
	renderWindowInteractor->Start();
	std::cout << "Renderer started" << std::endl;
}

/**
 * Main entry point
 */
int main( int argc, char * argv[] ) {
	Args args{ argc, argv};

	Field * field = load_field( args );

	for( int i=0; i<30; i++ ) 
		field->smooth_once();

	render_field( field );
	std::cout << "Done" << std::endl;
	delete field;
    return 0;
}
