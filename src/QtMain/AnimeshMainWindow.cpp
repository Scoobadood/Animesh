#include <QtWidgets>

#include "AnimeshMainWindow.h"
#include "ui_AnimeshMainWindow.h"

#include "vtkActor.h"
#include "vtkCallbackCommand.h"
#include "vtkCamera.h"
#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkCoordinate.h"
#include "vtkCylinderSource.h"
#include "vtkDataArray.h"
#include "vtkDepthSortPolyData.h"
#include "vtkFloatArray.h"
#include "vtkInteractorStyleTrackballCamera.h"
#include "vtkLight.h"
#include "vtkLine.h"
#include "vtkNamedColors.h"
#include "vtkPointData.h"
#include "vtkPolyDataMapper.h"
#include "vtkProperty.h"
#include "vtkRenderer.h"
#include "vtkRendererCollection.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkSphereSource.h"
#include "vtkTextActor.h"
#include "vtkTextProperty.h"
#include "vtkTextRepresentation.h"
#include "vtkTexturedButtonRepresentation2D.h"
#include "vtkTextWidget.h"
#include "vtkUnsignedCharArray.h"

namespace animesh {


AnimeshMainWindow::AnimeshMainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::AnimeshMainWindow) {
    m_field = nullptr;
    m_field_optimiser = nullptr;
    m_polydata = nullptr;
    m_current_tier = 0;

    ui->setupUi(this);

    disable_inspector( );

    // VTK/Qt wedded
    ui->qvtkWidget->GetRenderWindow()->AddRenderer(set_up_renderer( ));

    // Set up action signals and slots
    connect(this->ui->action_exit, SIGNAL(triggered()), this, SLOT(slotExit()));
}

AnimeshMainWindow::~AnimeshMainWindow() {
    delete ui;
}


/**
 * Handle the open file action
 */
void AnimeshMainWindow::on_action_open_triggered() {
    // Notify the app to load the file
    QString fileName = QFileDialog::getOpenFileName(this);
    if (!fileName.isEmpty()) {
        loadFile(fileName);
    }
}

//
// On select prefab:
// -- Create field
// -- Populate inspector
// -- update render view
void AnimeshMainWindow::on_action_poly_triggered() {
    set_field( Field::polynomial_field( 10, 10, 2.5, 5) );
}

void AnimeshMainWindow::on_action_plane_triggered() {
    set_field( Field::planar_field( 10, 10, 2.5, 5) );
}

/** Up or down a level in the graph
 */
void AnimeshMainWindow::on_sbGraphLevel_valueChanged(int new_graph_level) {
    m_current_tier = new_graph_level;
    view_changed( );
}

void AnimeshMainWindow::on_btnSmoothCompletely_clicked() {
    assert( m_field );
    assert( m_field_optimiser );

    m_field_optimiser->optimise();
    m_current_tier = 0;
    view_changed();
}

void AnimeshMainWindow::on_btnSmoothOnce_clicked() {
    assert( m_field );
    assert( m_field_optimiser );

    m_field_optimiser->optimise_once();
    m_current_tier = m_field_optimiser->optimising_tier_index( );
    view_changed();
}

/**
 * Load a new file, setup all the stuff
 */
void AnimeshMainWindow::loadFile( QString fileName ) {
    Field * field = load_field_from_obj_file( fileName.toStdString(), 5, 100, true );
    if( field ) {
        set_field( field );
        setWindowFilePath(fileName);
        statusBar()->showMessage(tr("File loaded"), 2000);
    } else {
        statusBar()->showMessage(tr("Error loading file"), 2000);
    }
}

/**
 * Reset all old variables
 * Set field to new variable
 * Refresh the view
 */
void AnimeshMainWindow::set_field( Field * new_field ) {
    if( m_field_optimiser != nullptr ) delete m_field_optimiser;
    if( m_field != nullptr ) delete m_field;

    // Set new values
    int m_current_tier = 0;
    m_field = new_field;
    m_field_optimiser = new FieldOptimiser( m_field );
    view_changed( );
}

//
// View changed
// -- Populate inspector
// -- update render view
void AnimeshMainWindow::view_changed()
{
    update_inspector();
    update_view();
}


//
// On load:
// -- Load field (or fail)
// -- Populate inspector
// -- update render view



/**
 * Disable all the input fields and buttons in the inspector
 */
void AnimeshMainWindow::disable_inspector( ) {
    this->ui->sbGraphLevel->setMaximum( 0 );
    this->ui->sbGraphLevel->setValue( 0 );
    this->ui->sbGraphLevel->setEnabled( false );
    this->ui->txtResidual->setText( "0" );
    this->ui->txtResidual->setEnabled( false );
    this->ui->txtNodeCount->setText( "0" );
    this->ui->txtNodeCount->setEnabled( false );
    this->ui->txtEdgeCount->setText( "0" );
    this->ui->txtEdgeCount->setEnabled( false );

    this->ui->btnSmoothOnce->setEnabled( false );
    this->ui->btnSmoothCompletely->setEnabled( false );
}

// Populate inspector
// -- Get the number of graph levels and set the 
//    min and max values of the spinner
//    set current level to bottom
//    updte spinner value
// -- Set the node count with nodes in this level
// -- Set the edge count with edges in this level
void AnimeshMainWindow::update_inspector(){
    std::cout << "Update inspector. field : " << m_field << std::endl;
    if( m_field == nullptr ) {
        disable_inspector( );
        return;
    }

    if( m_field_optimiser != nullptr ) {
        this->ui->btnSmoothOnce->setEnabled( true );
        this->ui->btnSmoothCompletely->setEnabled( true );

        // FO->num_tiers cannot be 0 at this stage
        if( m_field_optimiser->num_tiers( ) == 1 ) {
            this->ui->sbGraphLevel->setEnabled( false );
        } else {
            this->ui->sbGraphLevel->setEnabled( true );
        }
        this->ui->sbGraphLevel->setMaximum( m_field_optimiser->num_tiers( ) - 1);

        std::cout << "New field has " <<  m_field_optimiser->num_tiers( ) << " tiers" << std::endl;
        this->ui->sbGraphLevel->setValue( m_current_tier );

        this->ui->txtNodeCount->setText( QString::number(m_field_optimiser->graph_at_tier(m_current_tier)->num_nodes() ) );
        this->ui->txtEdgeCount->setText( QString::number(m_field_optimiser->graph_at_tier(m_current_tier)->num_edges() ) );

        this->ui->txtResidual->setText( QString::number( m_field_optimiser->current_error( m_current_tier) ) );
    }

}

// Update render view
// -- Rebuild/build new poly from field current layer
// -- Update the render window
void AnimeshMainWindow::update_view(){
    std::cout << "render view for graph at level: " << this->ui->sbGraphLevel->value( ) << std::endl;
    update_poly_data( );
    ui->qvtkWidget->GetRenderWindow()->Render();
}

/**
 * Reconstruct the given polydata from the field
 */
void AnimeshMainWindow::update_poly_data( ) {
    using namespace Eigen;

    vtkSmartPointer<vtkPoints>      pts  = vtkSmartPointer<vtkPoints>::New();
    vtkSmartPointer<vtkCellArray>   lines = vtkSmartPointer<vtkCellArray>::New();

    // Create a vtkUnsignedCharArray container and store the colors in it
    vtkSmartPointer<vtkNamedColors> named_colours = vtkSmartPointer<vtkNamedColors>::New();
    vtkSmartPointer<vtkUnsignedCharArray> colours = vtkSmartPointer<vtkUnsignedCharArray>::New();
    colours->SetNumberOfComponents(3);

    m_polydata->Initialize();
    if( m_field_optimiser != nullptr ) {
        FieldGraph * fg = m_field_optimiser->graph_at_tier( m_current_tier );
        for (auto gn : fg->nodes( ) ) {
            const FieldElement * const fe = gn->data();

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
          Vector3f p5 = location + (normal * 0.1);
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
            colours->InsertNextTypedTuple(named_colours->GetColor3ub("Yellow").GetData());
        }
    }
    m_polydata->SetPoints(pts);
    m_polydata->SetLines(lines);
    m_polydata->GetCellData()->SetScalars(colours);
}

/**
 * Draw the field
 */
vtkSmartPointer<vtkRenderer> AnimeshMainWindow::set_up_renderer( ) {
    vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();

    //  Make empty Polydata
    m_polydata = vtkSmartPointer<vtkPolyData>::New();
    update_poly_data();


// The depthSort object is set up to generate scalars representing
// the sort depth.  A camera is assigned for the sorting. The camera
// define the sort vector (position and focal point).
    vtkSmartPointer<vtkDepthSortPolyData> depth_sort = vtkSmartPointer<vtkDepthSortPolyData>::New();
    depth_sort->SetInputData(m_polydata);
    depth_sort->SetDirectionToBackToFront();
    depth_sort->SetVector(1, 1, 1);
    depth_sort->SetCamera(renderer->GetActiveCamera());
    depth_sort->SortScalarsOn();
    depth_sort->Update();

    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputConnection(depth_sort->GetOutputPort());

    mapper->SetScalarVisibility(true);
    mapper->SetScalarRange(0, depth_sort->GetOutput()->GetNumberOfCells());
    // mapper->SetInputData(m_polydata);

    vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);
    actor->GetProperty()->SetPointSize(3); 
    actor->GetProperty()->SetLineWidth(3);
    actor->GetProperty()->SetOpacity(0.5);
    actor->GetProperty()->SetColor(1, 0, 0);
    renderer->AddActor(actor);
    return renderer;
}



}