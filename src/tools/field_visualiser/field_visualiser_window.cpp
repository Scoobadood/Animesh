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
  connect(ui->btnDecimate, &QPushButton::clicked,
          this, [&]() {
        m_quad_geometry_extractor->collapse();
        extract_geometry();
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
  std::vector<std::pair<unsigned int, unsigned int>> red_edges;
  std::vector<std::pair<unsigned int, unsigned int>> blue_edges;
  m_quad_geometry_extractor->extract_geometry(m_graph_ptr, vertices, red_edges, blue_edges);
  ui->quadGLWidget->setData(vertices, red_edges, blue_edges);
}

void
field_visualiser_window::set_graph(SurfelGraphPtr graph_ptr) {
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
