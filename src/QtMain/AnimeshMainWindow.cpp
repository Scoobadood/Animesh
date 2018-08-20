#include <QtWidgets>

#include "AnimeshMainWindow.h"
#include <Field/FieldFactory.h>

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

using animesh::Field;
using animesh::FieldOptimiser;
using animesh::FieldElement;
using animesh::FieldFactory;
using animesh::FieldGraph;
using animesh::load_field_from_obj_file;

AnimeshMainWindow::AnimeshMainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::AnimeshMainWindow) {
    m_field = nullptr;
    m_field_optimiser = nullptr;
    m_polydata = nullptr;
    m_current_tier = 0;
    m_current_frame = 0;

    ui->setupUi(this);

    disable_inspector();

    // VTK/Qt wedded
    ui->qvtkWidget->GetRenderWindow()->AddRenderer(set_up_renderer());

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
        load_model_from_file(fileName);
    }
}

//
// On select prefab:
// -- Create field
// -- Populate inspector
// -- update render view
void AnimeshMainWindow::on_action_poly_triggered() {
    set_field(FieldFactory::polynomial_field(10, 10, 1, 5));
}

void AnimeshMainWindow::on_action_plane_triggered() {
    set_field(FieldFactory::planar_field(10, 10, 1, 5));
}

void AnimeshMainWindow::on_action_add_frame_triggered() {
    // Notify the app to load the file
    QString file_name = QFileDialog::getOpenFileName(this);
    if (!file_name.isEmpty()) {
        load_new_frame(file_name);
    }
}

/** Up or down a level in the graph
*/
void AnimeshMainWindow::on_sbGraphLevel_valueChanged(int new_graph_level) {
    m_current_tier = new_graph_level;
    view_changed();
}

/** Up or down a level in the graph
*/
void AnimeshMainWindow::on_sbFrameNumber_valueChanged(int new_frame_number) {
    // Frame should be zero based, dispaly is 1 based
    if ( m_current_frame != new_frame_number - 1 ) {
        m_current_frame = new_frame_number - 1;
        view_changed();
    }
}

void AnimeshMainWindow::on_btnSmoothCompletely_clicked() {
    assert(m_field);
    assert(m_field_optimiser);

    m_field_optimiser->optimise();
    m_current_tier = 0;
    view_changed();
}

void AnimeshMainWindow::on_btnSmoothOnce_clicked() {
    assert(m_field);
    assert(m_field_optimiser);

    m_field_optimiser->optimise_one_step();
    m_current_tier = m_field_optimiser->optimising_tier_index();
    view_changed();
}

void AnimeshMainWindow::on_cbMainTangent_stateChanged(int enabled) {
    std::cout << "Toggle main tangent " << enabled << std::endl;
    if (!enabled) {
        m_main_tangents_actor->VisibilityOff();
    }
    else {
        m_main_tangents_actor->VisibilityOn();
    }
    ui->qvtkWidget->GetRenderWindow()->Render();
}

void AnimeshMainWindow::on_cbSecondaryTangents_stateChanged(int enabled)
{
    std::cout << "Toggle secondary tangent " << enabled << std::endl;
    if (!enabled) {
        m_other_tangents_actor->VisibilityOff();
    }
    else {
        m_other_tangents_actor->VisibilityOn();
    }
    ui->qvtkWidget->GetRenderWindow()->Render();
}

void AnimeshMainWindow::on_cbNormals_stateChanged(int enabled)
{
    std::cout << "Toggle normals " << enabled << std::endl;
    if (!enabled) {
        m_normals_actor->VisibilityOff();
    }
    else {
        m_normals_actor->VisibilityOn();
    }
    ui->qvtkWidget->GetRenderWindow()->Render();
}

void AnimeshMainWindow::on_cbNeighbours_stateChanged(int enabled)
{
    std::cout << "Toggle neighbours " << enabled << std::endl;
    if (!enabled) {
        m_neighbours_actor->VisibilityOff();
    }
    else {
        m_neighbours_actor->VisibilityOn();
    }
    ui->qvtkWidget->GetRenderWindow()->Render();
}


/**
 * Load a new file, setup all the stuff
 */
void AnimeshMainWindow::load_model_from_file(QString fileName) {
    Field *field = load_field_from_obj_file(fileName.toStdString(), 5, false);
    if (field) {
        set_field(field);
        statusBar()->showMessage(tr("File loaded"), 2000);
    } else {
        statusBar()->showMessage(tr("Error loading file"), 2000);
    }
}

/**
 * Load a new file, setup all the stuff
 */
void AnimeshMainWindow::load_new_frame(QString file_name) {
    pcl::PointCloud<pcl::PointNormal>::Ptr cloud = animesh::load_pointcloud_from_obj(file_name.toStdString());
    if (cloud) {
        m_field->add_new_frame(cloud);
        update_frame_counter();
        statusBar()->showMessage(tr("Added frame"), 2000);
    } else {
        statusBar()->showMessage(tr("Error loading file"), 2000);
    }
}

/**
 * Compute the lengths to use for normals and tangent vectors based on the
 * mean length of an edge in the graph
 */
void AnimeshMainWindow::compute_scale() {
    assert( m_field != nullptr);
    float sum = 0.0f;
    for ( auto edge : m_field->m_graph->edges()) {
        Eigen::Vector3f v1 = edge->from_node( )->data()->location();
        Eigen::Vector3f v2 = edge->to_node( )->data()->location();
        Eigen::Vector3f diff = v2 - v1;
        sum = sum + diff.norm();
    }
    tan_scale_factor = (sum / m_field->m_graph->num_edges()) / 2.5f;
}
/**
 * Reset all old variables
 * Set field to new variable
 * Refresh the view
 */
void AnimeshMainWindow::set_field(Field *new_field) {
    // Cler out the old
    if (m_field_optimiser != nullptr) delete m_field_optimiser;
    if (m_field != nullptr) delete m_field;

    // Set new values
    int m_current_tier = 0;
    m_field = new_field;
    m_field_optimiser = new FieldOptimiser(m_field);
    ui->action_add_frame->setEnabled(m_field != nullptr);
    compute_scale();
    update_frame_counter();
    view_changed();
}

//
// View changed
// -- Populate inspector
// -- update render view
void AnimeshMainWindow::view_changed() {
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
void AnimeshMainWindow::disable_inspector() {
    this->ui->sbGraphLevel->setMaximum(0);
    this->ui->sbGraphLevel->setValue(0);
    this->ui->sbGraphLevel->setEnabled(false);

    disable_frame_counter();

    this->ui->txtResidual->setText("0");
    this->ui->txtResidual->setEnabled(false);
    this->ui->txtNodeCount->setText("0");
    this->ui->txtNodeCount->setEnabled(false);
    this->ui->txtEdgeCount->setText("0");
    this->ui->txtEdgeCount->setEnabled(false);

    this->ui->btnSmoothOnce->setEnabled(false);
    this->ui->btnSmoothCompletely->setEnabled(false);
}


void AnimeshMainWindow::disable_frame_counter( ) {
    ui->sbFrameNumber->setMaximum(0);
    ui->sbFrameNumber->setValue(0);
    ui->sbFrameNumber->setEnabled(false);
}

void AnimeshMainWindow::update_frame_counter( ) {
    if ( m_field != nullptr ) {
        int num_frames = m_field->get_num_frames();
        // If there are no (other) frames
        if ( num_frames == 0 ) {
            disable_frame_counter();
        }
        // Otherwise ...
        else {
            // We start numbering at 1.
            ui->sbFrameNumber->setMinimum(1);
            ui->sbFrameNumber->setMaximum(num_frames);
            if ( m_current_frame >= num_frames) {
                m_current_frame = 0;
                ui->sbFrameNumber->setValue(m_current_frame + 1);
            }
            if ( num_frames > 0 ) {
                ui->sbFrameNumber->setEnabled(true);
            } else {
                ui->sbFrameNumber->setEnabled(false);
            }
        }
    } else {
        disable_frame_counter();
    }
}

// Populate inspector
// -- Get the number of graph levels and set the
//    min and max values of the spinner
//    set current level to bottom
//    updte spinner value
// -- Set the node count with nodes in this level
// -- Set the edge count with edges in this level
void AnimeshMainWindow::update_inspector() {
    std::cout << "Update inspector. field : " << m_field << std::endl;
    if (m_field == nullptr) {
        disable_inspector();
        return;
    }

    // If there's an optimiser, there's a field to render
    if (m_field_optimiser != nullptr) {
        ui->btnSmoothOnce->setEnabled(true);
        ui->btnSmoothCompletely->setEnabled(true);

        // FO->num_tiers cannot be 0 at this stage
        if (m_field_optimiser->num_tiers() == 1) {
            ui->sbGraphLevel->setEnabled(false);
        } else {
            ui->sbGraphLevel->setEnabled(true);
        }
        ui->sbGraphLevel->setMaximum(m_field_optimiser->num_tiers() - 1);

        std::cout << "New field has " << m_field_optimiser->num_tiers() << " tiers" << std::endl;
        ui->sbGraphLevel->setValue(m_current_tier);

        ui->txtNodeCount->setText(
            QString::number(m_field_optimiser->graph_at_tier(m_current_tier)->num_nodes()));
        ui->txtEdgeCount->setText(
            QString::number(m_field_optimiser->graph_at_tier(m_current_tier)->num_edges()));

        ui->txtResidual->setText(QString::number(m_field_optimiser->current_error(m_current_tier)));
    }
}


// Update render view
// -- Rebuild/build new poly from field current layer
// -- Update the render window
void AnimeshMainWindow::update_view() {
    std::cout << "render view for graph at level: " << ui->sbGraphLevel->value() << std::endl;
    update_poly_data();
    ui->qvtkWidget->GetRenderWindow()->Render();
}

/**
 * Init the main tangent vector layer
 */
void AnimeshMainWindow::init_neighbours_layer( vtkSmartPointer<vtkRenderer> renderer ) {
    m_polydata_neighbours = vtkSmartPointer<vtkPolyData>::New();

    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputData(m_polydata_neighbours);

    m_neighbours_actor = vtkSmartPointer<vtkActor>::New();
    m_neighbours_actor->SetMapper(mapper);
    m_neighbours_actor->GetProperty()->SetPointSize(3);
    m_neighbours_actor->GetProperty()->SetLineWidth(3);
    m_neighbours_actor->GetProperty()->SetOpacity(1.0);
    renderer->AddActor(m_neighbours_actor);
}

/**
 * Update the secondary tangents layer
 */
void AnimeshMainWindow::update_neighbours_layer( ) {
    using namespace Eigen;
    using namespace std;

    vtkSmartPointer<vtkPoints> pts = vtkSmartPointer<vtkPoints>::New();
    vtkSmartPointer<vtkCellArray> lines = vtkSmartPointer<vtkCellArray>::New();

    // Create a vtkUnsignedCharArray container and store the colors in it
    vtkSmartPointer<vtkNamedColors> named_colours = vtkSmartPointer<vtkNamedColors>::New();
    vtkSmartPointer<vtkUnsignedCharArray> colours = vtkSmartPointer<vtkUnsignedCharArray>::New();
    colours->SetNumberOfComponents(3);

    m_polydata_neighbours->Initialize();
    if (m_field_optimiser != nullptr) {
        // Get the *graph*
        FieldGraph * graph = m_field_optimiser->graph_at_tier(m_current_tier);

        // Get each GN
        for ( auto gn : graph->nodes()) {
            const FieldElement * this_fe = m_field_optimiser->get_corresponding_fe_in_frame(m_current_frame, m_current_tier, gn->data());
            Vector3f location = this_fe->location();

            vector<FieldElement *> neighbours_at_frame0 = graph->neighbours_data( gn );
            vector<FieldElement *> neighbours = m_field_optimiser->get_corresponding_fes_in_frame(m_current_frame, m_current_tier, neighbours_at_frame0 );

            size_t num_points = neighbours.size() + 1;
            vtkIdType pid[num_points];
            pid[0] = pts->InsertNextPoint(location.x(), location.y(), location.z());
            size_t i = 1;
            for ( auto other_fe : neighbours ) {
                pid[i++] = pts->InsertNextPoint(other_fe->location().x(), other_fe->location().y(), other_fe->location().z());
            }
            // Main tangent
            vtkSmartPointer<vtkLine> line = vtkSmartPointer<vtkLine>::New();
            for (size_t i = 1; i < num_points; ++i) {
                line = vtkSmartPointer<vtkLine>::New();
                line->GetPointIds()->SetId(0, pid[0]);
                line->GetPointIds()->SetId(1, pid[i]);
                lines->InsertNextCell(line);
                colours->InsertNextTypedTuple(named_colours->GetColor3ub("Green").GetData());
            }
        }
    }
    m_polydata_neighbours->SetPoints(pts);
    m_polydata_neighbours->SetLines(lines);
    m_polydata_neighbours->GetCellData()->SetScalars(colours);
}

/**
 * Init the main tangent vector layer
 */
void AnimeshMainWindow::init_secondary_tangent_vector_layer( vtkSmartPointer<vtkRenderer> renderer ) {
    m_polydata_other_tangents = vtkSmartPointer<vtkPolyData>::New();

    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputData(m_polydata_other_tangents);

    m_other_tangents_actor = vtkSmartPointer<vtkActor>::New();
    m_other_tangents_actor->SetMapper(mapper);
    m_other_tangents_actor->GetProperty()->SetPointSize(3);
    m_other_tangents_actor->GetProperty()->SetLineWidth(3);
    m_other_tangents_actor->GetProperty()->SetOpacity(1.0);
    m_other_tangents_actor->GetProperty()->SetColor(1, 0, 0);
    renderer->AddActor(m_other_tangents_actor);
}

/**
 * Update the secondary tangents layer
 */
void AnimeshMainWindow::update_secondary_tangent_vector_layer( ) {
    using namespace Eigen;

    vtkSmartPointer<vtkPoints> pts = vtkSmartPointer<vtkPoints>::New();
    vtkSmartPointer<vtkCellArray> lines = vtkSmartPointer<vtkCellArray>::New();

    // Create a vtkUnsignedCharArray container and store the colors in it
    vtkSmartPointer<vtkNamedColors> named_colours = vtkSmartPointer<vtkNamedColors>::New();
    vtkSmartPointer<vtkUnsignedCharArray> colours = vtkSmartPointer<vtkUnsignedCharArray>::New();
    colours->SetNumberOfComponents(3);

    m_polydata_other_tangents->Initialize();
    if (m_field_optimiser != nullptr) {
        std::vector<FieldElement*> elements = m_field_optimiser->get_elements_at( m_current_frame, m_current_tier);
        for ( FieldElement * fe : elements ) {
            Vector3f location = fe->location();
            Vector3f normal = fe->normal();
            Vector3f tangent = fe->tangent();

            // Add the field node
            vtkIdType pid[4];

            // Add location
            pid[0] = pts->InsertNextPoint(location.x(), location.y(), location.z());

            // Add opposite tangent points
            Vector3f p2 = location - (tangent * tan_scale_factor);
            pid[1] = pts->InsertNextPoint(p2.x(), p2.y(), p2.z());

            // Compute 90 tangent
            Vector3f ninety = tangent.cross(normal);
            Vector3f p3 = location + (ninety * tan_scale_factor);
            pid[2] = pts->InsertNextPoint(p3.x(), p3.y(), p3.z());
            Vector3f p4 = location - (ninety * tan_scale_factor);
            pid[3] = pts->InsertNextPoint(p4.x(), p4.y(), p4.z());

            // Main tangent
            vtkSmartPointer<vtkLine> line = vtkSmartPointer<vtkLine>::New();
            for (int i = 0; i < 3; ++i) {
                line = vtkSmartPointer<vtkLine>::New();
                line->GetPointIds()->SetId(0, pid[0]);
                line->GetPointIds()->SetId(1, pid[i + 1]);
                lines->InsertNextCell(line);
                colours->InsertNextTypedTuple(named_colours->GetColor3ub("Pink").GetData());
            }
        }
    }
    m_polydata_other_tangents->SetPoints(pts);
    m_polydata_other_tangents->SetLines(lines);
    m_polydata_other_tangents->GetCellData()->SetScalars(colours);
}

/**
 * Init the main tangent vector layer
 */
void AnimeshMainWindow::init_main_tangent_vector_layer( vtkSmartPointer<vtkRenderer> renderer ) {
    m_polydata_main_tangents = vtkSmartPointer<vtkPolyData>::New();

    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputData(m_polydata_main_tangents);

    m_main_tangents_actor = vtkSmartPointer<vtkActor>::New();
    m_main_tangents_actor->SetMapper(mapper);
    m_main_tangents_actor->GetProperty()->SetPointSize(3);
    m_main_tangents_actor->GetProperty()->SetLineWidth(3);
    m_main_tangents_actor->GetProperty()->SetOpacity(1.0);
    m_main_tangents_actor->GetProperty()->SetColor(1, 0, 0);
    renderer->AddActor(m_main_tangents_actor);
}

/**
 * Update the main tangent layer
 */
void AnimeshMainWindow::update_main_tangent_vector_layer( ) {
    using namespace Eigen;

    vtkSmartPointer<vtkPoints> pts = vtkSmartPointer<vtkPoints>::New();
    vtkSmartPointer<vtkCellArray> lines = vtkSmartPointer<vtkCellArray>::New();

    // Create a vtkUnsignedCharArray container and store the colors in it
    vtkSmartPointer<vtkNamedColors> named_colours = vtkSmartPointer<vtkNamedColors>::New();
    vtkSmartPointer<vtkUnsignedCharArray> colours = vtkSmartPointer<vtkUnsignedCharArray>::New();
    colours->SetNumberOfComponents(3);

    m_polydata_main_tangents->Initialize();
    if (m_field_optimiser != nullptr) {
        std::vector<FieldElement*> elements = m_field_optimiser->get_elements_at( m_current_frame, m_current_tier);
        for ( FieldElement * fe : elements ) {
            Vector3f location = fe->location();
            Vector3f normal = fe->normal();
            Vector3f tangent = fe->tangent();

            // Add the field node
            vtkIdType pid[2];

            // Add location
            pid[0] = pts->InsertNextPoint(location.x(), location.y(), location.z());

            // Add tangent points
            Vector3f p1 = location + (tangent * tan_scale_factor);
            pid[1] = pts->InsertNextPoint(p1.x(), p1.y(), p1.z());

            // Main tangent
            vtkSmartPointer<vtkLine> line = vtkSmartPointer<vtkLine>::New();
            line->GetPointIds()->SetId(0, pid[0]);
            line->GetPointIds()->SetId(1, pid[1]);
            lines->InsertNextCell(line);
            colours->InsertNextTypedTuple(named_colours->GetColor3ub("Red").GetData());
        }
    }
    m_polydata_main_tangents->SetPoints(pts);
    m_polydata_main_tangents->SetLines(lines);
    m_polydata_main_tangents->GetCellData()->SetScalars(colours);
}

/**
 * Init the normal polydata/mapper/actor
 */
void AnimeshMainWindow::init_normals_layer( vtkSmartPointer<vtkRenderer> renderer ) {
    m_polydata_normals = vtkSmartPointer<vtkPolyData>::New();

    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputData(m_polydata_normals);

    m_normals_actor = vtkSmartPointer<vtkActor>::New();
    m_normals_actor->SetMapper(mapper);
    m_normals_actor->GetProperty()->SetPointSize(3);
    m_normals_actor->GetProperty()->SetLineWidth(3);
    m_normals_actor->GetProperty()->SetOpacity(1.0);
    m_normals_actor->GetProperty()->SetColor(1, 0, 0);
    renderer->AddActor(m_normals_actor);
}

/**
 * Update the normals layer
 */
void AnimeshMainWindow::update_normals_layer( ) {
    using namespace Eigen;

    vtkSmartPointer<vtkPoints> pts = vtkSmartPointer<vtkPoints>::New();
    vtkSmartPointer<vtkCellArray> lines = vtkSmartPointer<vtkCellArray>::New();

    // Create a vtkUnsignedCharArray container and store the colors in it
    vtkSmartPointer<vtkNamedColors> named_colours = vtkSmartPointer<vtkNamedColors>::New();
    vtkSmartPointer<vtkUnsignedCharArray> colours = vtkSmartPointer<vtkUnsignedCharArray>::New();
    colours->SetNumberOfComponents(3);

    m_polydata_normals->Initialize();
    if (m_field_optimiser != nullptr) {
        std::vector<FieldElement*> elements = m_field_optimiser->get_elements_at( m_current_frame, m_current_tier);
        for ( FieldElement * fe : elements ) {
            Vector3f location = fe->location();
            Vector3f normal = fe->normal();
            Vector3f tangent = fe->tangent();

            vtkIdType pid[2];
            // Add location
            pid[0] = pts->InsertNextPoint(location.x(), location.y(), location.z());

            // Add end of normal
            Vector3f n = location + (normal * tan_scale_factor * 0.2);
            pid[1] = pts->InsertNextPoint(n.x(), n.y(), n.z());

            // Line between them
            vtkSmartPointer<vtkLine> line = vtkSmartPointer<vtkLine>::New();
            line->GetPointIds()->SetId(0, pid[0]);
            line->GetPointIds()->SetId(1, pid[1]);
            lines->InsertNextCell(line);
            colours->InsertNextTypedTuple(named_colours->GetColor3ub("Yellow").GetData());
        }
    }
    m_polydata_normals->SetPoints(pts);
    m_polydata_normals->SetLines(lines);
    m_polydata_normals->GetCellData()->SetScalars(colours);
}



/**
 * Reconstruct the given polydata from the field
 */
void AnimeshMainWindow::update_poly_data() {
    update_normals_layer();
    update_main_tangent_vector_layer();
    update_secondary_tangent_vector_layer( );
    update_neighbours_layer( );
}

/**
 * Draw the field
 */
vtkSmartPointer<vtkRenderer> AnimeshMainWindow::set_up_renderer() {

    vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();

    init_normals_layer( renderer );
    init_main_tangent_vector_layer( renderer );
    init_secondary_tangent_vector_layer( renderer );
    init_neighbours_layer( renderer );

    update_poly_data( );

    return renderer;
}