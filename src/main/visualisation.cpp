#include <Field/Field.h>
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
#include "vtkCellData.h"
#include "vtkProperty.h"
#include "vtkDataArray.h"
#include "vtkLine.h"
#include "vtkUnsignedCharArray.h"
#include "vtkButtonWidget.h"
#include "vtkCoordinate.h"
#include "vtkTexturedButtonRepresentation2D.h"
#include "vtkCallbackCommand.h"
#include "vtkCamera.h"
#include "vtkNamedColors.h"
#include "vtkTextWidget.h"
#include "vtkTextActor.h"
#include "vtkTextProperty.h"
#include "vtkTextRepresentation.h"



// Global field declaration
static Field * g_field;

/**
 * Forwad declaration for the field
 */
void update_field_callback(vtkObject* caller,
                long unsigned int eventId,
                void* clientData, void* callData );


/**
 * Create a text widget to act as a button for incremental smoothing
 */
vtkSmartPointer<vtkTextWidget> add_smooth_button( vtkSmartPointer<vtkRenderWindowInteractor> interactor ) {
 // Create the widget
  vtkSmartPointer<vtkTextActor> textActor = vtkSmartPointer<vtkTextActor>::New();
  textActor->SetInput("Smooth Once");
  textActor->GetTextProperty()->SetColor( 0.0, 1.0, 0.0 );

  vtkSmartPointer<vtkTextWidget> textWidget = vtkSmartPointer<vtkTextWidget>::New();
  vtkSmartPointer<vtkTextRepresentation> textRepresentation = vtkSmartPointer<vtkTextRepresentation>::New();
  textRepresentation->GetPositionCoordinate()->SetValue( .15, .15 );
  textRepresentation->GetPosition2Coordinate()->SetValue( .7, .2 );
  textWidget ->SetRepresentation( textRepresentation );

  textWidget->SetInteractor(interactor);
  textWidget->SetTextActor(textActor);
  textWidget->SelectableOff();

  return textWidget;
}


/**
 * Reconstruct the given polydata from the field
 */
void populate_poly_from_field( const Field * const field, vtkSmartPointer<vtkPolyData> polydata ) {
	using namespace Eigen;

	vtkSmartPointer<vtkPoints>     	pts  = vtkSmartPointer<vtkPoints>::New();
    vtkSmartPointer<vtkCellArray>	lines = vtkSmartPointer<vtkCellArray>::New();


	// Create a vtkUnsignedCharArray container and store the colors in it
	vtkSmartPointer<vtkNamedColors> named_colours = vtkSmartPointer<vtkNamedColors>::New();
	vtkSmartPointer<vtkUnsignedCharArray> colours = vtkSmartPointer<vtkUnsignedCharArray>::New();
	colours->SetNumberOfComponents(3);

    polydata->Initialize();
    const std::vector<const FieldElement *> elements = field->elements();
    for (auto it = elements.begin(); it != elements.end(); ++it ) {
    	const FieldElement * const fe = *it;

    	Vector3f location = fe->m_location;
    	Vector3f tangent = fe->m_tangent;
    	Vector3f normal = fe->m_normal;

    	// Add the field node
    	vtkIdType pid[6];

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
        Vector3f p5 = location + normal;
        pid[5] = pts->InsertNextPoint(p5.x(), p5.y(), p5.z());

		// Main tangent
    	vtkSmartPointer<vtkLine> line = vtkSmartPointer<vtkLine>::New();
		line->GetPointIds()->SetId(0,pid[0]);
		line->GetPointIds()->SetId(1,pid[1]);
		lines->InsertNextCell(line);
		colours->InsertNextTypedTuple(named_colours->GetColor3ub("Red").GetData());

		// Secondary tangents
		for( int i=0; i<3; ++i ) {
	    	line = vtkSmartPointer<vtkLine>::New();
			line->GetPointIds()->SetId(0,pid[0]);
			line->GetPointIds()->SetId(1,pid[i + 2]);
			lines->InsertNextCell(line);
			colours->InsertNextTypedTuple(named_colours->GetColor3ub("Pink").GetData());
		}

		// Normal
    	line = vtkSmartPointer<vtkLine>::New();
		line->GetPointIds()->SetId(0,pid[0]);
		line->GetPointIds()->SetId(1,pid[5]);
		lines->InsertNextCell(line);
		colours->InsertNextTypedTuple(named_colours->GetColor3ub("White").GetData());
    }
    polydata->SetPoints(pts);
	polydata->SetLines(lines);

	vtkCellData * cd = polydata->GetCellData();
 	polydata->GetCellData()->SetScalars(colours);
}


/**
 * Draw the field
 */
vtkSmartPointer<vtkPolyData> set_up_render_field( const Field * const field  ) {
	vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
  renderer->GetActiveCamera()->Dolly( 2.0f );

  vtkSmartPointer<vtkRenderWindow> renderWindow = vtkSmartPointer<vtkRenderWindow>::New();
  renderWindow->SetSize(1240, 960);
  renderWindow->AddRenderer(renderer);

  vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
  renderWindowInteractor->SetRenderWindow(renderWindow);


  //  MAKE POLY DATA
  vtkSmartPointer<vtkPolyData> polydata = vtkSmartPointer<vtkPolyData>::New();
  vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  mapper->SetInputData(polydata);
  vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
  actor->SetMapper(mapper);
  actor->GetProperty()->SetPointSize(3); 
	renderer->AddActor(actor);


  //  Handle Callback
  vtkSmartPointer<vtkCallbackCommand> callback = vtkSmartPointer<vtkCallbackCommand>::New();
  callback->SetCallback(update_field_callback);
  callback->SetClientData((void *)polydata);
  renderWindowInteractor->AddObserver(vtkCommand::UserEvent, callback );

  vtkSmartPointer<vtkTextWidget> text = add_smooth_button(renderWindowInteractor);

  renderWindow->Render();

  text->On();

	populate_poly_from_field( field, polydata);

	renderWindowInteractor->Start();
	std::cout << "Renderer started" << std::endl;
  return polydata;
}

/**
 * Update the field by smoothing and redraw it.
 */
void update_field_callback(vtkObject* caller, long unsigned int eventId, void * clientData, void * callData ) {
  std::cout << "update_field_callback called (smooth 10)" << std::endl;
  for( int i=0; i<10; ++i)
    g_field->smooth_once();
  vtkPolyData* polydatap = reinterpret_cast<vtkPolyData*>(clientData);
  vtkSmartPointer<vtkPolyData> polydata = polydatap;
  populate_poly_from_field( g_field, polydata);

  vtkRenderWindowInteractor * rwi = (vtkRenderWindowInteractor *)caller;
  vtkRenderWindow * rw = rwi->GetRenderWindow();
  rw->Render();
}


void start_renderer( Field * field  ) {
  // Cache a copy
  g_field = field;
	set_up_render_field( field );
}
