#include <QFileDialog>
#include <Qmenu>
#include <QCoreApplication>
#include <QAbstractButton>
#include <QPushButton>
#include <QSlider>
#include <QtConcurrent/QtConcurrent>
#include <utility>
#include <ArcBall/TrackBall.h>

#include <Surfel/Surfel_IO.h>
#include "AnimeshWindow.h"
#include "AnimeshApp.h"
#include "AnimeshGLWidget.h"

#include "ui_AnimeshWindow.h"

AnimeshWindow::AnimeshWindow(QWidget *parent) //
    : QMainWindow{parent} //
    , ui{new Ui::AnimeshWindow} //
    , m_graph{nullptr} //
    , m_multi_res_graph{nullptr}//
    , m_scale_factor{1.0f} //
{
  ui->setupUi(this);

  connect(ui->menuFile, &QMenu::triggered, this, &AnimeshWindow::fileOpenAction);

  m_arc_ball = std::make_shared<Trackball>();
  ui->animeshGLWidget->set_arc_ball(m_arc_ball);

  connect(ui->cbShowNormals, &QCheckBox::toggled,
          ui->animeshGLWidget, &AnimeshGLWidget::toggle_normals);
  connect(ui->cbShowTangents, &QCheckBox::toggled,
          ui->animeshGLWidget, &AnimeshGLWidget::toggle_tangents);
  connect(ui->cbShowMainTangents, &QCheckBox::toggled,
          ui->animeshGLWidget, &AnimeshGLWidget::toggle_main_tangents);

  connect(ui->btnSolve, &QPushButton::clicked, this, &AnimeshWindow::start_solving);
  connect(ui->slScale, &QSlider::valueChanged, this, &AnimeshWindow::change_scale);

  auto &random = ((AnimeshApp *) QCoreApplication::instance())->random_engine();
  m_field_optimiser = std::make_unique<FieldOptimiser>(random, 10, 1.0f);
}

AnimeshWindow::~AnimeshWindow() {
  delete ui;
}

void
AnimeshWindow::change_scale(int value) {
  ui->animeshGLWidget->set_scale(static_cast<float>(value + 1) * m_scale_factor);
}

/*
 * Compute the mean inter-neighbour distance and use it
 * to adjust the scale slider.
 */
void
AnimeshWindow::reset_scale_factor() {
  if (m_graph == nullptr) {
    return;
  }
  int count = 0;
  float total = total;
  for (const auto &node: m_graph->nodes()) {
    Eigen::Vector3f t, n, v;
    node->data()->get_vertex_tangent_normal_for_frame(0, v, t, n);
    const auto neighbours = get_node_neighbours_in_frame(m_graph, node, 0);
    for (const auto &nbr: neighbours) {
      Eigen::Vector3f tn, nn, vn;
      nbr->data()->get_vertex_tangent_normal_for_frame(0, vn, tn, nn);
      total += ((vn - v).norm());
      ++count;
    }
  }
  auto mean_dist = (total / (float) count);
  // We'd like the ability to scale up to overlap a tangent with a neighbour
  // So at '10' the length of a tangent should be more than the mean inter-node distance
  m_scale_factor = mean_dist / 8.0f;
}

void
AnimeshWindow::set_graph(SurfelGraphPtr &graph) {
  m_graph = graph;
  auto rng = ((AnimeshApp *) QCoreApplication::instance())->random_engine();
  m_multi_res_graph = std::make_shared<MultiResolutionSurfelGraph>(graph, rng);
  m_field_optimiser->set_graph(m_multi_res_graph);
  m_arc_ball->reset();
  reset_scale_factor();
  change_scale(ui->slScale->value());
  update();
}

void
AnimeshWindow::fileOpenAction() {
  auto fileName = QFileDialog::getOpenFileName(this,
                                               tr("Open Graph"), "",
                                               tr("Surfel Graph Files (*.bin);;All Files (*)"));
  if (fileName.isNull() || fileName.isEmpty()) {
    return;
  }

  auto app = (AnimeshApp *) (QCoreApplication::instance());
  auto graphPtr = load_surfel_graph_from_file(fileName.toStdString(), app->random_engine());
  set_graph(graphPtr);
}

void AnimeshWindow::start_solving() {
  if (m_graph == nullptr) {
    return;
  }
  if (m_field_optimiser == nullptr) {
    return;
  }
  ui->btnSolve->setDisabled(true);
  QtConcurrent::run([&](){
    while(!m_field_optimiser->optimise_once()) {
      ui->animeshGLWidget->update();
    }
    ui->btnSolve->setEnabled(true);
  });
}