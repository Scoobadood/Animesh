#include "field_visualiser_window.h"
#include "ui_field_visualiser_window.h"
#include <Surfel/SurfelGraph.h>
#include <Surfel/Surfel_IO.h>
#include <QFileDialog>
#include <utility>
#include <ArcBall/TrackBall.h>
#include <QString>
field_visualiser_window::field_visualiser_window(Properties properties,
                                                 std::default_random_engine &rng,
                                                 QWidget *parent) //
    : QMainWindow(parent) //
    , m_random_engine{rng} //
    , ui(new Ui::field_visualiser_window) //
    , m_selected_node{nullptr} //
    , m_selected_node_is_visible{false} //
    , m_properties{std::move(properties)} //
{
  float rho = m_properties.getFloatProperty("rho");
  m_posy_geometry_extractor = new posy_surfel_graph_geometry_extractor(rho);
  m_rosy_geometry_extractor = new rosy_surfel_graph_geometry_extractor();
  m_quad_geometry_extractor = new quad_geometry_extractor(rho);
  m_arc_ball = std::make_shared<Trackball>();

  ui->setupUi(this);

  ui->posyGLWidget->setRho(rho);
  ui->posyGLWidget->set_arc_ball(m_arc_ball);
  ui->rosyGLWidget->set_arc_ball(m_arc_ball);
  ui->quadGLWidget->set_arc_ball(m_arc_ball);

  connect(ui->menuFile, &QMenu::triggered,
          this, &field_visualiser_window::fileOpenAction);
  connect(ui->frameSelector, &QSlider::valueChanged,
          this, &field_visualiser_window::frameChanged);
  connect(ui->splatSizeSelector, &QSlider::valueChanged,
          [=](int value) {
            float mapped_value = (float) value / 10.0f;
            m_posy_geometry_extractor->set_splat_scale_factor(0.5f + mapped_value);
            extract_geometry();
          });
  connect(ui->cbShowQuads, &QCheckBox::toggled, ui->posyGLWidget, &posy_gl_widget::render_quads);
  connect(ui->cbShowTextures, &QCheckBox::toggled, ui->posyGLWidget, &posy_gl_widget::render_textures);
  connect(ui->cbShowTriangleFans, &QCheckBox::toggled, ui->posyGLWidget, &posy_gl_widget::render_triangle_fans);

  connect(ui->cbNormals, &QCheckBox::toggled,
          ui->rosyGLWidget, &rosy_gl_widget::renderNormals);
  connect(ui->cbMainTangent, &QCheckBox::toggled,
          ui->rosyGLWidget, &rosy_gl_widget::renderMainTangents);
  connect(ui->cbOtherTangents, &QCheckBox::toggled,
          ui->rosyGLWidget, &rosy_gl_widget::renderOtherTangents);
  connect(ui->cbErrorColours, &QCheckBox::toggled,
          ui->rosyGLWidget, &rosy_gl_widget::renderErrorColours);
  connect(ui->cbShowSplats, &QCheckBox::toggled,
          ui->rosyGLWidget, &rosy_gl_widget::renderSplats);
  connect(ui->cbShowPath, &QCheckBox::toggled,
          ui->rosyGLWidget, &rosy_gl_widget::renderPath);
  connect(ui->cbShowNeighbours, &QCheckBox::toggled, ui->rosyGLWidget, &rosy_gl_widget::renderNeighbours);
  connect(ui->slNormalLength, &QSlider::valueChanged,
          this, [&](int value) {
        float spur_length;
        switch (value) {
          case 1:spur_length = 0.01f;
            break;
          case 2:spur_length = 0.05f;
            break;
          case 3:spur_length = 0.1f;
            break;
          case 4:spur_length = 0.2f;
            break;
          case 5:spur_length = 0.3f;
            break;
          case 6:spur_length = 0.4f;
            break;
          case 7:spur_length = 0.5f;
            break;
          case 8:spur_length = 0.75f;
            break;
          case 9:spur_length = 1.0f;
            break;
          case 10:spur_length = 2.0f;
            break;
          default:spur_length = 1.0f;
            break;
        }
        m_rosy_geometry_extractor->set_spur_length(spur_length);
        extract_geometry();
      });

  connect(ui->cbBlue, &QCheckBox::toggled,
          ui->quadGLWidget, &quad_gl_widget::showBlueEdges);
  connect(ui->cbRed, &QCheckBox::toggled,
          ui->quadGLWidget, &quad_gl_widget::showRedEdges);
  connect(ui->cbAffinities, &QCheckBox::toggled,
          ui->quadGLWidget, &quad_gl_widget::showVertexAffinities);
  connect(ui->btnDecimate, &QPushButton::clicked,
          this, [&]() {
        m_quad_geometry_extractor->collapse();
        extract_geometry(false);
      });

  connect(ui->quadGLWidget, &quad_gl_widget::edge_selected, this, [&](std::string &from_name, std::string &to_name) {
    using namespace std;
  });
  connect(ui->quadGLWidget, &quad_gl_widget::no_edge_selected,
          this, [&]() {
        using namespace std;
      });

  m_timer = new QTimer(this); //Create a timer
  m_timer->callOnTimeout([=]() {
    ui->rosyGLWidget->update();
    ui->posyGLWidget->update();
    ui->quadGLWidget->update();
  });
  m_timer->start(30);
}

field_visualiser_window::~field_visualiser_window() {
  delete m_posy_geometry_extractor;
  delete m_rosy_geometry_extractor;
  delete m_quad_geometry_extractor;
  delete ui;
}

void
field_visualiser_window::extract_geometry(bool rebuild_edge_graph) {
  using namespace std;

  std::vector<float> positions;
  std::vector<float> tangents;
  std::vector<float> colours;
  std::vector<float> quads;
  std::vector<float> triangle_fans;
  std::vector<float> triangle_uvs;
  std::vector<unsigned int> triangle_fan_sizes;
  std::vector<float> normals;
  std::vector<float> path;
  std::vector<float> splat_sizes;
  std::vector<float> uvs;
  float scale_factor;

  m_posy_geometry_extractor->extract_geometry(
      m_graph_ptr,
      positions,
      quads,
      triangle_fans,
      triangle_uvs,
      triangle_fan_sizes,
      normals,
      splat_sizes,
      uvs);
  ui->posyGLWidget->setPoSyData(positions, quads,
                                triangle_fans,
                                triangle_uvs,
                                triangle_fan_sizes,
                                normals, splat_sizes, uvs);

  m_rosy_geometry_extractor->extract_geometry(
      m_graph_ptr,
      positions,
      tangents,
      normals,
      colours,
      path,
      triangle_fans,
      triangle_fan_sizes,
      scale_factor
  );
  ui->rosyGLWidget->setRoSyData(positions,
                                normals,
                                tangents,
                                colours,
                                path,
                                triangle_fans,
                                triangle_fan_sizes,
                                scale_factor);

  std::vector<float> vertices;
  std::vector<std::pair<std::pair<std::string, unsigned int>, std::pair<std::string, unsigned int>>> red_edges;
  std::vector<std::pair<std::pair<std::string, unsigned int>, std::pair<std::string, unsigned int>>> blue_edges;
  std::vector<float> original_vertices;
  std::vector<float> vertex_affinity;
  m_quad_geometry_extractor->extract_geometry(vertices,
                                              red_edges,
                                              blue_edges,
                                              original_vertices,
                                              vertex_affinity,
                                              rebuild_edge_graph);
  m_edge_from_node_names.clear();
  ui->quadGLWidget->setData(vertices, red_edges, blue_edges, original_vertices, vertex_affinity);

  // FIXME:
  // This code associates surfel IDs with nodes in edge graph
  // BUT we want something else. When the quad graph gets decimated, it's the nodes from *that* that we care about.
  // So seems like we should populate edge from node names form Quad graph node names
  // But QG node names come from initial surfel graph. Read through this code
  // And figure out WTF.
  for (auto &edge: m_graph_ptr->edges()) {
    pair<string, string> key1 = {edge.from()->data()->id(), edge.to()->data()->id()};
    pair<string, string> key2 = {edge.to()->data()->id(), edge.from()->data()->id()};

    m_edge_from_node_names.emplace(key1, edge.data());
    m_edge_from_node_names.emplace(key2, edge.data());
  }
}

void
field_visualiser_window::set_graph(SurfelGraphPtr graph_ptr) {
  using namespace std;

  // Deselect any selected node.
  m_selected_node = nullptr;

  m_quad_geometry_extractor->set_graph(graph_ptr);
  m_graph_ptr = std::move(graph_ptr);
  extract_geometry();

  auto num_frames = get_num_frames(m_graph_ptr);
  ui->frameSelector->setValue(0);
  ui->frameSelector->setMinimum(0);
  ui->frameSelector->setMaximum((int) num_frames - 1);
}

void field_visualiser_window::fileOpenAction() {
  QString fileName = QFileDialog::getOpenFileName(this,
                                                  tr("Open Graph"), "",
                                                  tr("Surfel Graph Files (*.bin);;All Files (*)"));
  if( fileName.isNull() || fileName.isEmpty()) {
    return;
  }

  const auto graphPtr = load_surfel_graph_from_file(fileName.toStdString(),
                                                    m_random_engine);
  set_graph(graphPtr);
}

void field_visualiser_window::frameChanged(int value) {
  m_rosy_geometry_extractor->set_frame(value);
  m_posy_geometry_extractor->set_frame(value);
  m_quad_geometry_extractor->set_frame(value);
  ui->lblFrame->setText(std::to_string(value).c_str());
  extract_geometry();
}

void field_visualiser_window::quad_vertex_selected(int i) {
}
