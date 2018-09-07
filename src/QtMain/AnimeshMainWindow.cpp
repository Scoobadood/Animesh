#include <QtWidgets>
#include <Eigen/Geometry>

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
#include "vtkDoubleArray.h"
#include "vtkFloatArray.h"
#include "vtkInteractorStyleTrackballCamera.h"
#include "vtkLight.h"
#include "vtkLine.h"
#include "vtkNamedColors.h"
#include "vtkOpenGLPolyDataMapper.h"
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
using animesh::FieldGraph;

AnimeshMainWindow::AnimeshMainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::AnimeshMainWindow) {
    m_field = nullptr;
    m_field_optimiser = nullptr;
    m_polydata_main_tangents = nullptr;
    m_polydata_other_tangents = nullptr;
    m_polydata_normals = nullptr;
    m_polydata_neighbours = nullptr;
    m_main_tangents_actor = nullptr;
    m_other_tangents_actor = nullptr;
    m_normals_actor = nullptr;
    m_neighbours_actor = nullptr;
    m_current_tier = 0;
    m_current_frame = 0;
    m_draw_main_tangent = true;
    m_draw_other_tangents = true;
    m_draw_normals = false;
    m_draw_neighbours = false;

    ui->setupUi(this);

    // VTK/Qt wedded
    ui->qvtkWidget->GetRenderWindow()->AddRenderer(set_up_renderer());

    // Set up action signals and slots
    connect(ui->action_exit, SIGNAL(triggered()), this, SLOT(slotExit()));

    init_UI();
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
    if ( new_graph_level == m_current_tier + 1)
        return;

    m_current_tier = new_graph_level - 1;
    update_view();
}

void AnimeshMainWindow::on_btnSmoothCompletely_clicked() {
    assert(m_field);
    assert(m_field_optimiser);

    m_field_optimiser->optimise();
    update_view();
}

void AnimeshMainWindow::on_btnSmoothOnce_clicked() {
    assert(m_field);
    assert(m_field_optimiser);

    ui->btnRandomise->setEnabled(false);
    m_field_optimiser->optimise_one_step();
    set_current_tier(m_field_optimiser->optimising_tier_index());
    if (!m_field_optimiser->is_optimising()) {
        ui->btnRandomise->setEnabled(true);
    }
}

void 
AnimeshMainWindow::update_view_layers() {
    m_draw_main_tangent ? m_main_tangents_actor->VisibilityOn() : m_main_tangents_actor->VisibilityOff();
    m_draw_other_tangents ? m_other_tangents_actor->VisibilityOn() : m_other_tangents_actor->VisibilityOff();
    m_draw_normals ? m_normals_actor->VisibilityOn() : m_normals_actor->VisibilityOff();
    m_draw_neighbours ? m_neighbours_actor->VisibilityOn() : m_neighbours_actor->VisibilityOff();
    ui->qvtkWidget->GetRenderWindow()->Render();
}

void 
AnimeshMainWindow::on_cbMainTangent_stateChanged(int enabled) {
    m_draw_main_tangent = ui->cbMainTangent->isChecked();
    update_view_layers();
}

void 
AnimeshMainWindow::on_cbSecondaryTangents_stateChanged(int enabled) {
    m_draw_other_tangents = ui->cbSecondaryTangents->isChecked();
    update_view_layers();
}

void 
AnimeshMainWindow::on_cbNormals_stateChanged(int enabled) {
    m_draw_normals = ui->cbNormals->isChecked();
    update_view_layers();
}

void 
AnimeshMainWindow::on_cbNeighbours_stateChanged(int enabled) {
    m_draw_neighbours = ui->cbNeighbours->isChecked();
    update_view_layers();
}


void AnimeshMainWindow::on_hs_frame_selector_valueChanged(int new_frame_idx) {
    set_current_frame( new_frame_idx - 1 );
}

void AnimeshMainWindow::on_cb_include_frame_stateChanged(int enabled) {
    if( m_field_optimiser == nullptr ) return;
    if( m_current_frame == 0 ) return;

    m_field_optimiser->enable_frame(m_current_frame, ui->cb_include_frame->isChecked());
}

void AnimeshMainWindow::on_btnRandomise_clicked() {
    m_field_optimiser->randomise();
    update_metrics();
    update_view();
}

/* ********************************************************************************
 *
 *   Load a new file, setup all the stuff
 *
 * ********************************************************************************/

/**
 *
 */
void AnimeshMainWindow::set_current_frame( size_t new_frame_idx ) {
    if ( m_current_frame == new_frame_idx )
        return;

    if ( new_frame_idx == 0 ) {
        m_current_frame = 0;
    } else {
        assert(m_field != nullptr);
        assert(new_frame_idx < m_field->get_num_frames());
        m_current_frame = new_frame_idx;
    }
    update_include_checkbox();
    update_frame_selector_value();
    update_view();
}

void AnimeshMainWindow::set_current_tier( size_t new_tier_idx ) {
    if ( m_current_tier == new_tier_idx )
        return;

    m_current_tier = new_tier_idx;
    update_graph_tier_selector();
    update_metrics();
    update_view();
}



/**
 * Load a new file, setup all the stuff
 */
void AnimeshMainWindow::load_model_from_file(QString file_name) {
    ObjFileParser parser{file_name.toStdString()};
    Field *field = new Field( parser.m_vertices, parser.m_normals, parser.m_adjacency );
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
    ObjFileParser parser{file_name.toStdString()};
    m_field_optimiser->add_new_frame(parser.m_vertices, parser.m_normals);
    update_frame_selector_range( );
    statusBar()->showMessage(tr("Added frame"), 2000);
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
    m_field = new_field;
    m_field_optimiser = new FieldOptimiser(m_field);

    enable_buttons();
    enable_display_checkboxes();
    compute_scale();
    update_include_checkbox();
    set_current_tier(0);
    update_metrics();
    update_view();
}

/* ********************************************************************************
 *
 *   Initialise UI
 *
 * ********************************************************************************/

void
AnimeshMainWindow::disable_graph_level() {
    ui->sbGraphLevel->setMinimum(1);
    ui->sbGraphLevel->setMaximum(1);
    ui->sbGraphLevel->setValue(1);
    ui->sbGraphLevel->setEnabled(false);
}

void
AnimeshMainWindow::disable_frame_selector() {
    ui->hs_frame_selector->setMinimum(1);
    ui->hs_frame_selector->setMaximum(1);
    ui->hs_frame_selector->setValue(1);
    ui->lbl_first_frame->setText("1");
    ui->lbl_last_frame->setText("1");
    ui->lbl_current_frame->setText("1");
}

void
AnimeshMainWindow::clear_metrics() {
    ui->txtResidual->setText("0");
    ui->txtResidual->setEnabled(false);
    ui->txtNodeCount->setText("0");
    ui->txtNodeCount->setEnabled(false);
    ui->txtEdgeCount->setText("0");
    ui->txtEdgeCount->setEnabled(false);
}

void
AnimeshMainWindow::update_metrics() {
    if ( m_field_optimiser == nullptr ) return;

    ui->txtNodeCount->setText(
        QString::number(m_field_optimiser->graph_at_tier(m_current_tier)->num_nodes()));
    ui->txtEdgeCount->setText(
        QString::number(m_field_optimiser->graph_at_tier(m_current_tier)->num_edges()));
    ui->txtResidual->setText(QString::number(m_field_optimiser->current_error(m_current_tier)));
}

void
AnimeshMainWindow::update_graph_tier_selector( ) {
    if ( m_field_optimiser == nullptr ) return;

    if (m_field_optimiser->num_tiers() == 1) {
        ui->sbGraphLevel->setEnabled(false);
    } else {
        ui->sbGraphLevel->setEnabled(true);
    }
    ui->sbGraphLevel->setMaximum(m_field_optimiser->num_tiers());
    ui->sbGraphLevel->setValue(m_current_tier);
}

void 
AnimeshMainWindow::init_include_checkbox() {
    ui->cb_include_frame->setChecked( true );
    ui->cb_include_frame->setEnabled(false);
}

void 
AnimeshMainWindow::update_include_checkbox() {
    if( m_field == nullptr || m_current_frame == 0) {
        ui->cb_include_frame->setEnabled(false);
    }
    ui->cb_include_frame->setEnabled(false);
    ui->cb_include_frame->setChecked( m_field_optimiser->should_include_frame(m_current_frame) );
}

void
AnimeshMainWindow::disable_buttons() {
    ui->btnRandomise->setEnabled(false);
    ui->btnSmoothOnce->setEnabled(false);
    ui->btnSmoothCompletely->setEnabled(false);
}

void
AnimeshMainWindow::enable_buttons( ) {
    ui->btnRandomise->setEnabled(true);
    ui->btnSmoothOnce->setEnabled(true);
    ui->btnSmoothCompletely->setEnabled(true);
}

void
AnimeshMainWindow::disable_display_checkboxes() {
    ui->cbMainTangent->setEnabled(false);
    ui->cbSecondaryTangents->setEnabled(false);
    ui->cbNormals->setEnabled(false);
    ui->cbNeighbours->setEnabled(false);
}

void
AnimeshMainWindow::enable_display_checkboxes() {
    ui->cbMainTangent->setEnabled(true);
    ui->cbSecondaryTangents->setEnabled(true);
    ui->cbNormals->setEnabled(true);
    ui->cbNeighbours->setEnabled(true);
}

void
AnimeshMainWindow::init_display_checkboxes() {
    disable_display_checkboxes();
    ui->cbMainTangent->setChecked(true);
    ui->cbSecondaryTangents->setChecked(true);
    ui->cbNormals->setChecked(false);
    ui->cbNeighbours->setChecked(false);
}

void
AnimeshMainWindow::init_UI() {
    disable_graph_level();
    disable_frame_selector();
    clear_metrics();
    disable_buttons();
    update_view_layers();
    init_include_checkbox();
    init_display_checkboxes();
}

/* ********************************************************************************
 *
 *   Update UI
 *
 * ********************************************************************************/
void
AnimeshMainWindow::update_frame_selector_range( ) {
    size_t actual_num_frames = m_field->get_num_frames();
    size_t control_num_frames = ui->hs_frame_selector->maximum();
    if ( control_num_frames != actual_num_frames) {
        ui->hs_frame_selector->setMaximum(actual_num_frames);
        ui->lbl_last_frame->setText(QString::number(actual_num_frames));
        if ( m_current_frame >= actual_num_frames) {
            set_current_frame(actual_num_frames - 1);
        }
    }
}

void
AnimeshMainWindow::update_frame_selector_value( ) {
    ui->hs_frame_selector->setValue(m_current_frame + 1);
    ui->lbl_current_frame->setText(QString::number(m_current_frame + 1));
}

// Update render view
// -- Rebuild/build new poly from field current layer
// -- Update the render window
void AnimeshMainWindow::update_view() {
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

    vtkSmartPointer<vtkOpenGLPolyDataMapper> mapper = vtkSmartPointer<vtkOpenGLPolyDataMapper>::New();
    mapper->SetInputData(m_polydata_other_tangents);

    m_other_tangents_actor = vtkSmartPointer<vtkActor>::New();
    m_other_tangents_actor->SetMapper(mapper);
    m_other_tangents_actor->GetProperty()->SetPointSize(3);
    m_other_tangents_actor->GetProperty()->SetLineWidth(3);
    m_other_tangents_actor->GetProperty()->SetOpacity(1.0);
    mapper->AddShaderReplacement(
        vtkShader::Vertex,
        "//VTK::Normal::Dec", // replace the normal block
        true, // before the standard replacements
        "//VTK::Normal::Dec\n" // we still want the default
        "  varying vec3 myNormalNCVSOutput;\n", //but we add this
        false // only do it once
    );
    mapper->AddShaderReplacement(
        vtkShader::Vertex,
        "//VTK::Normal::Impl", // replace the normal block
        true, // before the standard replacements
        "//VTK::Normal::Impl\n" // we still want the default
        "  myNormalNCVSOutput = normalMC;\n", //but we add this
        false // only do it once
    );

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

        // TEST Try setting point normals for the tangent array
        vtkSmartPointer<vtkDoubleArray> pointNormalsArray = vtkSmartPointer<vtkDoubleArray>::New();
        pointNormalsArray->SetNumberOfComponents(3); //3d normals (ie x,y,z)
        pointNormalsArray->SetNumberOfTuples(elements.size() * 4);
        size_t i = 0;
        // TEST End

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

            // TEST Set 4 normals
            double pN1[3] = {normal[0], normal[1], normal[2]};
            pointNormalsArray->SetTuple(i++, pN1) ;
            pointNormalsArray->SetTuple(i++, pN1) ;
            pointNormalsArray->SetTuple(i++, pN1) ;
            pointNormalsArray->SetTuple(i++, pN1) ;
            // TEST End of Test code.




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
        m_polydata_other_tangents->GetPointData()->SetNormals(pointNormalsArray);
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