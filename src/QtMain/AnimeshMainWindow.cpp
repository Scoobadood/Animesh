#include <QtWidgets>
#include <Eigen/Geometry>

#include <FileUtils/FileUtils.h>
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

using animesh::FieldOptimiser;
using animesh::ObjFileParser;
using animesh::PointNormal;

const bool FACE_WISE = false;

std::vector<std::string> get_files_in_directory( std::string directory_name ) {
    using namespace std;

    vector<string> file_names;
    files_in_directory( directory_name, file_names, []( string name ) {
        using namespace std;

        std::transform(name.begin(), name.end(), name.begin(), ::tolower);
        string obj = ".obj";

        if( name.size() < 4 ) return false;
        return equal(obj.rbegin(), obj.rend(), name.rbegin());
    });
    // Construct full path names
    vector<string> full_path_names;
    for( string file_name : file_names) {
        // FIXME: Replace this evilness with something more robust and cross platform.
        string path_name = directory_name + "/" + file_name;
        full_path_names.push_back( path_name );
    }
    return full_path_names;
}

AnimeshMainWindow::AnimeshMainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::AnimeshMainWindow) {
    m_field_optimiser = nullptr;
    m_polydata_cross_field = nullptr;
    m_polydata_normals = nullptr;
    m_polydata_neighbours = nullptr;
    m_cross_field_actor = nullptr;
    m_normals_actor = nullptr;
    m_neighbours_actor = nullptr;
    m_current_tier = 0;
    m_current_frame = 0;
    m_draw_cross_field = true;
    m_highlight_main_tangent = true;
    m_draw_normals = false;
    m_draw_neighbours = false;

    ui->setupUi(this);

    // VTK/Qt wedded
    ui->qvtkWidget->GetRenderWindow()->AddRenderer(set_up_renderer());

    // Set up action signals and slots
    connect(ui->action_exit, SIGNAL(triggered()), this, SLOT(on_action_exit_triggered()));

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
    QFileDialog dialog{this};
    dialog.setFileMode(QFileDialog::Directory);
    dialog.setNameFilter("*.obj");
    QDir startDirectory{"/Users"};
    QStringList file_names;
    if (dialog.exec()) {
        file_names = dialog.selectedFiles();
        if (!file_names.isEmpty()) {
            load_from(file_names);
        }
    }
}

void AnimeshMainWindow::on_action_exit_triggered() {

}

/** Up or down a level in the graph
*/
void AnimeshMainWindow::on_sbGraphLevel_valueChanged(int new_graph_level) {
    if ( new_graph_level - 1 == m_current_tier)
        return;

    set_current_tier(new_graph_level - 1);
}

void AnimeshMainWindow::on_btnSmoothCompletely_clicked() {
    assert(m_field_optimiser);

    m_field_optimiser->optimise();
    maybe_update_singularities();
    update_metrics();
    update_view();
}

void AnimeshMainWindow::on_btnSmoothOnce_clicked() {
    assert(m_field_optimiser);

    ui->btnRandomise->setEnabled(false);
    m_field_optimiser->optimise_do_one_step();
    set_current_tier(m_field_optimiser->optimising_tier_index());
    maybe_update_singularities();
    update_graph_tier_selector();
    update_metrics();
    update_view();

    if (!m_field_optimiser->is_optimising()) {
        ui->btnRandomise->setEnabled(true);
    }
}

void
AnimeshMainWindow::update_view_layers() {
    m_draw_cross_field ? m_cross_field_actor->VisibilityOn() : m_cross_field_actor->VisibilityOff();
    m_draw_normals ? m_normals_actor->VisibilityOn() : m_normals_actor->VisibilityOff();
    m_draw_neighbours ? m_neighbours_actor->VisibilityOn() : m_neighbours_actor->VisibilityOff();
    m_draw_singularities ? m_singularities_actor->VisibilityOn() : m_singularities_actor->VisibilityOff();
    ui->qvtkWidget->GetRenderWindow()->Render();
}

void
AnimeshMainWindow::on_cbMainTangent_stateChanged(int enabled) {
    m_highlight_main_tangent = ui->cbMainTangent->isChecked();
    update_cross_field_layer();
    update_view_layers();
}

void
AnimeshMainWindow::on_cbSecondaryTangents_stateChanged(int enabled) {
    m_draw_cross_field = ui->cbSecondaryTangents->isChecked();
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

void
AnimeshMainWindow::on_cbSingularities_stateChanged(int enabled) {
    m_draw_singularities = (enabled == Qt::Checked);
    maybe_update_singularities( );
    update_metrics( );
    update_singularities_layer();
    update_view_layers( );
}

void AnimeshMainWindow::on_hs_frame_selector_valueChanged(int new_frame_idx) {
    set_current_frame( new_frame_idx - 1 );
}

void AnimeshMainWindow::on_cb_include_frame_stateChanged(int enabled) {
  if ( m_field_optimiser == nullptr ) return;
  bool should_be_checked = m_field_optimiser->is_frame_enabled(m_current_frame);
  if( (should_be_checked && (enabled == Qt::Checked )) ||
      (!should_be_checked && (enabled == Qt::Unchecked)) ) {
       return;
     }

  m_field_optimiser->enable_frame(m_current_frame, true);
  update_include_checkbox( );
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
 * Reset all controls once a file has been loaded.
 */
void
AnimeshMainWindow::file_loaded( ) {
    m_num_tiers = m_field_optimiser->num_tiers();
    m_num_frames = m_field_optimiser->num_frames();

    compute_scale( );
    enable_display_checkboxes( );
    enable_buttons();
    update_include_checkbox();
    update_frame_selector_range();
    update_frame_selector_value();
    update_graph_tier_selector();
    update_metrics();
    update_view();
}

/**
 * Load from one or more files or directory.
 * If file_names has more than one entry, they must all be files and we load them all
 * If it has one entry, it could be a directory or a file.
 */
void
AnimeshMainWindow::load_from( const QStringList& file_names ) {
    using namespace std;
    // Nothing
    if( file_names.size() == 0) return;

    // Either a single file or directory
    if( file_names.size() == 1 ) {
        QString file_name = file_names[0];
        bool is_directory  =false;
        if( file_exists( file_name.toStdString(), is_directory ) ) {
            if( is_directory ) {
                load_from_directory( file_name.toStdString() );
            } else {
                // Single file to open
                load_from_file( file_name.toStdString() );
            }
        }
        // else file didn't exist
    } else {
        // Load all files in QStringList
        vector<string> std_file_names;
        for( auto s : file_names ) {
            std_file_names.push_back( s.toStdString());
        }
        load_multiple_files(std_file_names);
    }
}

/**
 * Load multiple files
 */
void
AnimeshMainWindow::load_multiple_files( const std::vector<std::string>& file_names ) {
    using namespace std;

    // Sanity check
    assert( file_names.size() > 0 );
    for( string file_name : file_names ) {
        bool is_directory = false;
        assert (file_exists( file_name, is_directory ) && !is_directory );
    }

    ObjFileParser parser;

    vector<vector<PointNormal::Ptr>>    frames;
    multimap<size_t, size_t>            adjacency;
    pair<vector<PointNormal::Ptr>, multimap<size_t, size_t>> results = parser.parse_file( file_names[0], true, FACE_WISE );
    frames.push_back( results.first );
    adjacency = results.second;

    for( size_t file_idx = 1; file_idx < file_names.size(); ++file_idx ) {
        vector<PointNormal::Ptr> frame_data = parser.parse_file( file_names[file_idx], FACE_WISE ).first;
        frames.push_back(frame_data);
    }
    m_field_optimiser = new FieldOptimiser(frames, adjacency);
    file_loaded();
}

/**
 * Load from directory.
 */
void
AnimeshMainWindow::load_from_directory( const std::string& dir_name ) {
    using namespace std;

    vector<string> file_names = get_files_in_directory( dir_name );
    load_multiple_files( file_names );
}

/**
 * Load from directory.
 */
void
AnimeshMainWindow::load_from_file( const std::string& file_name ) {
    using namespace std;

    ObjFileParser parser;
    vector<vector<PointNormal::Ptr>>    frames;
    multimap<size_t, size_t>            adjacency;
    pair<vector<PointNormal::Ptr>, multimap<size_t, size_t>> results = parser.parse_file( file_name, true);
    frames.push_back( results.first );
    adjacency = results.second;
    m_field_optimiser = new FieldOptimiser(frames, adjacency);
    file_loaded();
}

/**
 *
 */
void AnimeshMainWindow::set_current_frame( size_t new_frame_idx ) {
    if ( m_current_frame == new_frame_idx )
        return;

    if ( new_frame_idx == 0 ) {
        m_current_frame = 0;
    } else {
        assert(new_frame_idx < m_field_optimiser->num_frames());
        m_current_frame = new_frame_idx;
    }
    update_include_checkbox();
    update_frame_selector_value();
    update_view();
}

/**
 * new_tier_idx is 0 .. num_tiers - 1
 */
void AnimeshMainWindow::set_current_tier( size_t new_tier_idx ) {
    if ( m_current_tier == new_tier_idx )
        return;

    m_current_tier = new_tier_idx;
    if( m_current_tier == 0 ) {
      enable_singularities_checkbox( );
    } else {
      disable_singularities_checkbox( );
    }
    update_graph_tier_selector();
    update_metrics();
    update_view();
}

/**
 * Compute the lengths to use for normals and tangent vectors based on the
 * mean length of an edge in the graph
 */
void AnimeshMainWindow::compute_scale() {
    float mean_edge_length = m_field_optimiser->mean_edge_length_for_tier(m_current_tier);
    tan_scale_factor = mean_edge_length / 2.5f;
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
        QString::number(m_field_optimiser->num_nodes_in_tier(m_current_tier)));
    ui->txtEdgeCount->setText(
        QString::number(m_field_optimiser->num_edges_in_tier(m_current_tier)));
    ui->txtResidual->setText(QString::number(m_field_optimiser->total_error()));

    if(m_draw_singularities) {
      ui->lblSingularities->setVisible(true);
      ui->txtSingRed->setVisible(true);
      ui->txtSingBlue->setVisible(true);
      ui->txtSingGreen->setVisible(true);
      ui->txtSingRed->setText(QString::number(m_num_red_singularities));
      ui->txtSingBlue->setText(QString::number(m_num_blue_singularities));
      ui->txtSingGreen->setText(QString::number(m_num_green_singularities));
    } else {
      ui->lblSingularities->setVisible(false);
      ui->txtSingRed->setVisible(false);
      ui->txtSingBlue->setVisible(false);
      ui->txtSingGreen->setVisible(false);
    }
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
    ui->sbGraphLevel->setValue(m_current_tier+1);
}

void
AnimeshMainWindow::init_include_checkbox() {
    ui->cb_include_frame->setChecked( true );
    ui->cb_include_frame->setEnabled(false);
}

void
AnimeshMainWindow::update_include_checkbox() {
  if( m_field_optimiser == nullptr ) return;

  bool frame_is_included = m_field_optimiser->is_frame_enabled(m_current_frame);
  ui->cb_include_frame->setChecked(frame_is_included);

  size_t frames_included = 0;
  for( size_t i=0; i<m_field_optimiser->num_frames(); ++i ) {
    if ( m_field_optimiser->is_frame_enabled(i) ) frames_included++;
  }

  if(( frames_included > 1) || !frame_is_included) {
    ui->cb_include_frame->setEnabled(true);
  } else {
    ui->cb_include_frame->setEnabled(false);
  }
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
    ui->cbSingularities->setEnabled(false);
}

void
AnimeshMainWindow::enable_display_checkboxes() {
    ui->cbMainTangent->setEnabled(true);
    ui->cbSecondaryTangents->setEnabled(true);
    ui->cbNormals->setEnabled(true);
    ui->cbNeighbours->setEnabled(true);
    ui->cbSingularities->setEnabled(true);
}

void
AnimeshMainWindow::enable_singularities_checkbox() {
    ui->cbSingularities->setEnabled(true);
}

void
AnimeshMainWindow::disable_singularities_checkbox() {
    ui->cbSingularities->setEnabled(false);
}

void
AnimeshMainWindow::init_display_checkboxes() {
    disable_display_checkboxes();
    ui->cbMainTangent->setChecked(true);
    ui->cbSecondaryTangents->setChecked(true);
    ui->cbNormals->setChecked(false);
    ui->cbNeighbours->setChecked(false);
    ui->cbSingularities->setEnabled(false);
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
    size_t actual_num_frames = m_field_optimiser->num_frames();
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
AnimeshMainWindow::maybe_update_singularities( ) {
  using namespace std;

  if( (m_field_optimiser == nullptr ) || (!m_draw_singularities)) {
    return;
  }
  // We only render singularities on the first tier.
  if( m_current_tier != 0 ) {
    return;
  }

  m_singularities = m_field_optimiser->get_singularities_for_tier_and_frame(m_current_tier, m_current_frame);
  m_num_red_singularities = m_num_blue_singularities = m_num_green_singularities = 0;
  for( auto s : m_singularities ) {
    size_t type =get<2>(s);
    switch( type ) {
      case 1:
        m_num_red_singularities++;
        break;
      case 2:
        m_num_green_singularities++;
        break;
      case 3:
        m_num_blue_singularities++;
        break;
      default:
        cerr << "Unrecognised singularity type " << type << endl;
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
    init_layer( renderer, m_polydata_neighbours, m_neighbours_actor);
    renderer->AddActor(m_neighbours_actor);
}

/**
 * Init the main tangent vector layer
 */
void AnimeshMainWindow::init_cross_field_layer( vtkSmartPointer<vtkRenderer> renderer ) {
    init_layer( renderer, m_polydata_cross_field, m_cross_field_actor);
    renderer->AddActor(m_cross_field_actor);
}

/**
 * Init the normal polydata/mapper/actor
 */
void
AnimeshMainWindow::init_normals_layer( vtkSmartPointer<vtkRenderer> renderer ) {
    init_layer( renderer, m_polydata_normals, m_normals_actor);
    renderer->AddActor(m_normals_actor);
}

/**
 * Init the singularities layer
 */
void
AnimeshMainWindow::init_singularities_layer( vtkSmartPointer<vtkRenderer> renderer ) {
  init_layer( renderer, m_polydata_singularities, m_singularities_actor);
  m_singularities_actor -> GetProperty()->SetPointSize(10);
  renderer->AddActor(m_singularities_actor);
}

void
AnimeshMainWindow::init_layer( vtkSmartPointer<vtkRenderer> renderer, vtkSmartPointer<vtkPolyData>& poly_data, vtkSmartPointer<vtkActor>& actor ) {
    poly_data = vtkSmartPointer<vtkPolyData>::New();
    vtkSmartPointer<vtkOpenGLPolyDataMapper> mapper = vtkSmartPointer<vtkOpenGLPolyDataMapper>::New();
    mapper->SetInputData(poly_data);
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
    actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);
    actor->GetProperty()->SetPointSize(3);
    actor->GetProperty()->SetLineWidth(3);
    actor->GetProperty()->SetOpacity(1.0);
}

/**
 * Update the secondary tangents layer
 */
void AnimeshMainWindow::update_neighbours_layer() {
    using namespace Eigen;
    using namespace std;

    vtkSmartPointer<vtkPoints> pts = vtkSmartPointer<vtkPoints>::New();
    vtkSmartPointer<vtkCellArray> lines = vtkSmartPointer<vtkCellArray>::New();
    vtkSmartPointer<vtkNamedColors> named_colours = vtkSmartPointer<vtkNamedColors>::New();
    vtkSmartPointer<vtkUnsignedCharArray> colours = vtkSmartPointer<vtkUnsignedCharArray>::New();
    colours->SetNumberOfComponents(3);

    m_polydata_neighbours->Initialize();

    if (m_field_optimiser != nullptr) {
        vector<PointNormal::Ptr> vertices = m_field_optimiser->point_normals_for_tier_and_frame(m_current_tier, m_current_frame);
        vector<vector<size_t>> adjacency = m_field_optimiser->adjacency_for_tier(m_current_tier);
        size_t num_vertices = vertices.size();

        vtkSmartPointer<vtkFloatArray>  vtk_point_normals = vtkSmartPointer<vtkFloatArray>::New();
        vtk_point_normals->SetNumberOfComponents(3); //3d normals (ie x,y,z)
        vtk_point_normals->SetNumberOfTuples(num_vertices);

        vtkIdType pid[num_vertices];
        size_t vertex_idx = 0;
        for ( auto vertex : vertices ) {
            Vector3f location = vertex->point();
            Vector3f normal   = vertex->normal();
            pid[vertex_idx] = pts->InsertNextPoint(location.x(), location.y(), location.z());
            vtk_point_normals->SetTuple(vertex_idx, normal.data()) ;
            vertex_idx++;
        }
        // Add lines between adjacent points
        vtkSmartPointer<vtkLine> line = vtkSmartPointer<vtkLine>::New();
        for (size_t i = 0; i < num_vertices; ++i) {
            vector<size_t> neighbours = adjacency[i];
            for ( size_t neighbour : neighbours ) {
                line = vtkSmartPointer<vtkLine>::New();
                line->GetPointIds()->SetId(0, pid[i]);
                line->GetPointIds()->SetId(1, pid[neighbour]);
                lines->InsertNextCell(line);
                colours->InsertNextTypedTuple(named_colours->GetColor3ub("Green").GetData());
            }
        }
        m_polydata_neighbours->GetPointData()->SetNormals(vtk_point_normals);

    }
    m_polydata_neighbours->SetPoints(pts);
    m_polydata_neighbours->SetLines(lines);
    m_polydata_neighbours->GetCellData()->SetScalars(colours);
}


/**
 * Update the secondary tangents layer
 */
void AnimeshMainWindow::update_cross_field_layer() {
    using namespace std;
    using namespace Eigen;

    vtkSmartPointer<vtkPoints> pts = vtkSmartPointer<vtkPoints>::New();
    vtkSmartPointer<vtkCellArray> lines = vtkSmartPointer<vtkCellArray>::New();
    vtkSmartPointer<vtkNamedColors> named_colours = vtkSmartPointer<vtkNamedColors>::New();
    vtkSmartPointer<vtkUnsignedCharArray> colours = vtkSmartPointer<vtkUnsignedCharArray>::New();
    colours->SetNumberOfComponents(3);

    m_polydata_cross_field->Initialize();
    if (m_field_optimiser != nullptr) {
        const vector<PointNormal::Ptr>& point_normals = m_field_optimiser->point_normals_for_tier_and_frame( m_current_tier, m_current_frame );
        vector<Vector3f>                tangents = m_field_optimiser->compute_tangents_for_tier_and_frame( m_current_tier, m_current_frame );
        size_t                          num_vertices = point_normals.size();

        // TEST Try setting point normals for the tangent array
        vtkSmartPointer<vtkFloatArray>  vtk_point_normals = vtkSmartPointer<vtkFloatArray>::New();
        size_t                          vtk_points_per_vertex = 5;
        size_t                          num_vtk_points = num_vertices * vtk_points_per_vertex;
        vtk_point_normals->SetNumberOfComponents(3); //3d normals (ie x,y,z)
        vtk_point_normals->SetNumberOfTuples(num_vtk_points);
        size_t vtk_point_normal_idx = 0;
        // TEST End

        for ( size_t vertex_idx = 0; vertex_idx < num_vertices; ++vertex_idx ) {
            // Add the field node
            Vector3f locations[vtk_points_per_vertex];
            Vector3f tangent = tangents[vertex_idx];
            Vector3f normal = point_normals[vertex_idx]->normal();

            locations[0] = point_normals[vertex_idx]->point();
            locations[1] = locations[0] + (tangent * tan_scale_factor);
            locations[2] = locations[0] - (tangent * tan_scale_factor);
            Vector3f ninety = tangent.cross(normal);
            locations[3] = locations[0] + (ninety * tan_scale_factor);
            locations[4] = locations[0] - (ninety * tan_scale_factor);

            vtkIdType pid[vtk_points_per_vertex];
            for ( size_t p_idx = 0; p_idx < vtk_points_per_vertex; p_idx ++ ) {
                pid[p_idx] = pts->InsertNextPoint(locations[p_idx].x(), locations[p_idx].y(), locations[p_idx].z());
                vtk_point_normals->SetTuple(vtk_point_normal_idx++, normal.data()) ;
            }
            vtkSmartPointer<vtkLine> line = vtkSmartPointer<vtkLine>::New();
            for (size_t line_idx = 1; line_idx < vtk_points_per_vertex; ++line_idx) {
                line = vtkSmartPointer<vtkLine>::New();
                line->GetPointIds()->SetId(0, pid[0]);
                line->GetPointIds()->SetId(1, pid[line_idx]);
                lines->InsertNextCell(line);
                // Optionally highlight the main

                unsigned char rgba[4];
                if ( (line_idx == 1) && m_highlight_main_tangent) {
                    named_colours->GetColor("red", rgba);
                } else {
                    named_colours->GetColor("grey", rgba);
                }
                colours->InsertNextTypedTuple(rgba);
            }
        }
        m_polydata_cross_field->GetPointData()->SetNormals(vtk_point_normals);
    }

    m_polydata_cross_field->SetPoints(pts);
    m_polydata_cross_field->SetLines(lines);
    m_polydata_cross_field->GetCellData()->SetScalars(colours);
}


/**
 * Update the normals layer
 */
void AnimeshMainWindow::update_normals_layer( ) {
    using namespace std;
    using namespace Eigen;

    vtkSmartPointer<vtkPoints> pts = vtkSmartPointer<vtkPoints>::New();
    vtkSmartPointer<vtkCellArray> lines = vtkSmartPointer<vtkCellArray>::New();
    vtkSmartPointer<vtkNamedColors> named_colours = vtkSmartPointer<vtkNamedColors>::New();
    vtkSmartPointer<vtkUnsignedCharArray> colours = vtkSmartPointer<vtkUnsignedCharArray>::New();
    colours->SetNumberOfComponents(3);

    m_polydata_normals->Initialize();
    if (m_field_optimiser != nullptr) {
        const vector<PointNormal::Ptr>& point_normals = m_field_optimiser->point_normals_for_tier_and_frame( m_current_tier, m_current_frame );
        size_t num_vertices = point_normals.size();

        // TEST Try setting point normals for the tangent array
        vtkSmartPointer<vtkFloatArray> vtk_point_normals = vtkSmartPointer<vtkFloatArray>::New();
        size_t num_vtk_points = point_normals.size() * 2;
        vtk_point_normals->SetNumberOfComponents(3); //3d normals (ie x,y,z)
        vtk_point_normals->SetNumberOfTuples(num_vtk_points);
        size_t vtk_point_normal_idx = 0;
        // TEST End

        for ( size_t vertex_idx = 0; vertex_idx < num_vertices; ++vertex_idx ) {
            Vector3f location = point_normals[vertex_idx]->point();
            Vector3f normal   = point_normals[vertex_idx]->normal();

            vtkIdType pid[num_vtk_points];
            pid[0] = pts->InsertNextPoint(location.x(), location.y(), location.z());
            Vector3f pt = location + (normal * tan_scale_factor * 0.2);
            pid[1] = pts->InsertNextPoint(pt.x(), pt.y(), pt.z());

            vtk_point_normals->SetTuple(vtk_point_normal_idx++, normal.data()) ;
            vtk_point_normals->SetTuple(vtk_point_normal_idx++, normal.data()) ;

            // Line between them
            vtkSmartPointer<vtkLine> line = vtkSmartPointer<vtkLine>::New();
            line->GetPointIds()->SetId(0, pid[0]);
            line->GetPointIds()->SetId(1, pid[1]);
            lines->InsertNextCell(line);
            colours->InsertNextTypedTuple(named_colours->GetColor3ub("Yellow").GetData());
        }
        m_polydata_normals->GetPointData()->SetNormals(vtk_point_normals);
    }
    m_polydata_normals->SetPoints(pts);
    m_polydata_normals->SetLines(lines);
    m_polydata_normals->GetCellData()->SetScalars(colours);
}

/**
 * Update the singularities layer
 */
void
AnimeshMainWindow::update_singularities_layer( ) {
    using namespace std;
    using namespace Eigen;

    vtkSmartPointer<vtkPoints> pts = vtkSmartPointer<vtkPoints>::New();
    vtkSmartPointer<vtkCellArray> vertices = vtkSmartPointer<vtkCellArray>::New();
    vtkSmartPointer<vtkNamedColors> named_colours = vtkSmartPointer<vtkNamedColors>::New();
    vtkSmartPointer<vtkUnsignedCharArray> colours = vtkSmartPointer<vtkUnsignedCharArray>::New();
    colours->SetNumberOfComponents(3);

    m_polydata_singularities->Initialize();
    if (m_field_optimiser != nullptr) {

        size_t num_vertices = m_singularities.size();

        vtkSmartPointer<vtkFloatArray> vtk_point_normals = vtkSmartPointer<vtkFloatArray>::New();
        size_t num_vtk_points = num_vertices;
        vtk_point_normals->SetNumberOfComponents(3); //3d normals (ie x,y,z)
        vtk_point_normals->SetNumberOfTuples(num_vtk_points);
        size_t vtk_point_normal_idx = 0;

        for ( auto singularity : m_singularities ) {
            Vector3f location = get<0>(singularity);
            Vector3f normal   = get<1>(singularity);
            int type          = get<2>(singularity);

            vtkIdType pid[num_vtk_points];
            pid[0] = pts->InsertNextPoint(location.x(), location.y(), location.z());
            vertices->InsertNextCell(1, pid);

            float nn[] = {0.0f, 0.0f, 1.0f};
            vtk_point_normals->SetTuple(vtk_point_normal_idx++, normal.data()) ;

            colours->InsertNextTypedTuple(named_colours->GetColor3ub(
                (type == 1) ? "Red" : (type == 2) ? "Green" : "Cyan").GetData());
        }
        m_polydata_singularities->GetPointData()->SetNormals(vtk_point_normals);
    }
    m_polydata_singularities->SetPoints(pts);
    m_polydata_singularities->SetVerts(vertices);
    m_polydata_singularities->GetCellData()->SetScalars(colours);
}




/**
 * Reconstruct the given polydata from the field
 */
void AnimeshMainWindow::update_poly_data() {
    update_normals_layer();
    update_cross_field_layer();
    update_neighbours_layer( );
    update_singularities_layer();
}

/**
 * Draw the field
 */
vtkSmartPointer<vtkRenderer> AnimeshMainWindow::set_up_renderer() {

    vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();

    init_normals_layer( renderer );
    init_cross_field_layer( renderer );
    init_neighbours_layer( renderer );
    init_singularities_layer( renderer);

    update_poly_data( );

    return renderer;
}
