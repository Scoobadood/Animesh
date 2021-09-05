#include "field_visualiser_window.h"
#include "posy_surfel_graph_geometry_extractor.h"
#include "ui_field_visualiser_window.h"
#include <Surfel/SurfelGraph.h>
#include <Surfel/Surfel_IO.h>
#include <QFileDialog>
#include <utility>

field_visualiser_window::field_visualiser_window(Properties properties, QWidget *parent) :
    QMainWindow(parent), ui(new Ui::field_visualiser_window), m_properties{std::move(properties)} {
  ui->setupUi(this);

  m_arc_ball = new ArcBall();

  float rho = m_properties.getFloatProperty("rho");
  ui->posyGLWidget->setRho(rho);
  ui->posyGLWidget->set_arc_ball(m_arc_ball);
  ui->rosyGLWidget->set_arc_ball(m_arc_ball);
  ui->quadGLWidget->set_arc_ball(m_arc_ball);

  m_posy_geometry_extractor = new posy_surfel_graph_geometry_extractor(rho);
  m_rosy_geometry_extractor = new rosy_surfel_graph_geometry_extractor();
  m_quad_geometry_extractor = new quad_geometry_extractor(rho);

  connect(ui->menuFile, &QMenu::triggered,
          this, &field_visualiser_window::fileOpenAction);
  connect(ui->frameSelector, &QSlider::valueChanged,
          this, &field_visualiser_window::frameChanged);
  connect(ui->splatSizeSelector, &QSlider::valueChanged,
          [=](int value) {
            float mapped_value = value / 10.0f;
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

  connect(ui->cbBlue, &QCheckBox::toggled,
          ui->quadGLWidget, &quad_gl_widget::showBlueEdges);
  connect(ui->cbRed, &QCheckBox::toggled,
          ui->quadGLWidget, &quad_gl_widget::showRedEdges);
  connect(ui->cbAffinities, &QCheckBox::toggled,
          ui->quadGLWidget, &quad_gl_widget::showVertexAffinities);
  connect(ui->btnDecimate, &QPushButton::clicked,
          this, [&]() {
        m_quad_geometry_extractor->collapse();
        extract_geometry();
      });

  connect(ui->quadGLWidget, &quad_gl_widget::edge_selected, this, [&](std::string &from_name, std::string &to_name) {
    using namespace std;

    ui->lblEdgeVertex1->setText(from_name.c_str());
    ui->lblEdgeVertex2->setText(to_name.c_str());
    // Extract the edge data
    auto edge = m_edge_from_node_names[{from_name, to_name}];
    auto t_ij_0 = edge->t_ij(0);
    auto t_ji_0 = edge->t_ji(0);
    auto k_ij_0 = edge->k_ij();
    auto k_ji_0 = edge->k_ji();
    QString t_ij{(to_string(t_ij_0.x()) + ", " + to_string(t_ij_0.y())).c_str()};
    QString t_ji{(to_string(t_ji_0.x()) + ", " + to_string(t_ji_0.y())).c_str()};
    ui->lblEdgeTij->setText(t_ij);
    ui->lblEdgeTji->setText(t_ji);
    ui->lblEdgeKij->setNum(k_ij_0);
    ui->lblEdgeKji->setNum(k_ji_0);
  });
  connect(ui->quadGLWidget, &quad_gl_widget::no_edge_selected,
          this, [&]() {
    using namespace std;

    ui->lblEdgeVertex1->setText("-");
    ui->lblEdgeVertex2->setText("-");
    // Extract the edge data
    ui->lblEdgeTij->setText("");
    ui->lblEdgeTji->setText("");
    ui->lblEdgeKij->setText("");
    ui->lblEdgeKji->setText("");
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
  delete m_arc_ball;
  delete ui;
}

void
field_visualiser_window::extract_geometry() {
  using namespace  std;

  std::vector<float> positions;
  std::vector<float> tangents;
  std::vector<float> colours;
  std::vector<float> quads;
  std::vector<float> triangle_fans;
  std::vector<float> triangle_uvs;
  std::vector<unsigned int> fan_sizes;
  std::vector<float> normals;
  std::vector<float> splat_sizes;
  std::vector<float> uvs;
  float scale_factor;

  m_posy_geometry_extractor->extract_geometry(
      m_graph_ptr,
      positions,
      quads,
      triangle_fans,
      triangle_uvs,
      fan_sizes,
      normals,
      splat_sizes,
      uvs);
  ui->posyGLWidget->setPoSyData(positions, quads,
                                triangle_fans,
                                triangle_uvs,
                                fan_sizes,
                                normals, splat_sizes, uvs);

  m_rosy_geometry_extractor->extract_geometry(
      m_graph_ptr,
      positions,
      tangents,
      normals,
      colours,
      scale_factor
  );
  ui->rosyGLWidget->setRoSyData(positions, normals, tangents, colours, scale_factor);

  std::vector<float> vertices;
  std::vector<std::pair<std::pair<std::string, unsigned int>, std::pair<std::string, unsigned int>>> red_edges;
  std::vector<std::pair<std::pair<std::string, unsigned int>, std::pair<std::string, unsigned int>>> blue_edges;
  std::vector<float> original_vertices;
  std::vector<float>vertex_affinity;
  m_quad_geometry_extractor->extract_geometry(vertices, red_edges, blue_edges, original_vertices, vertex_affinity);
  m_edge_from_node_names.clear();
  ui->quadGLWidget->setData(vertices, red_edges, blue_edges, original_vertices, vertex_affinity);

  // FIXME:
  // This code associates surfel IDs with nodes in edge graph
  // BUT we want something else. When the quad graph gets decimated, it's the nodes from *that* that we care about.
  // So seems like we should populate edge from node names form Quad graph node names
  // But QG node names come from initial surfel graph. Read through this code
  // And figure out WTF.
  for (auto &edge : m_graph_ptr->edges()) {
    pair<string, string> key1 = {edge.from()->data()->id(), edge.to()->data()->id()};
    pair<string, string> key2 = {edge.to()->data()->id(), edge.from()->data()->id()};

    m_edge_from_node_names.emplace(key1, edge.data());
    m_edge_from_node_names.emplace(key2, edge.data());
  }
}

void
field_visualiser_window::set_graph(SurfelGraphPtr graph_ptr) {
  using namespace std;

  m_graph_ptr = std::move(graph_ptr);
  m_quad_geometry_extractor->set_graph(m_graph_ptr);
  extract_geometry();
}

void field_visualiser_window::fileOpenAction() {
  QString fileName = QFileDialog::getOpenFileName(this,
                                                  tr("Open Graph"), "",
                                                  tr("Surfel Graph Files (*.bin);;All Files (*)"));
  const auto graphPtr = load_surfel_graph_from_file(fileName.toStdString());
  set_graph(graphPtr);
}

void field_visualiser_window::frameChanged(int value) {
  m_rosy_geometry_extractor->set_frame(value);
  m_posy_geometry_extractor->set_frame(value);
  extract_geometry();
}

void field_visualiser_window::quad_vertex_selected(int i) {

}
