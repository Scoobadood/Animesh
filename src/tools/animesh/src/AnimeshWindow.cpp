#include <QFileDialog>
#include <Qmenu>
#include <QCoreApplication>
#include <QAbstractButton>
#include <QPushButton>
#include <QSlider>
#include <QtConcurrent/QtConcurrent>
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
  connect(ui->cbShowPoSy, &QCheckBox::toggled,
          ui->animeshGLWidget, &AnimeshGLWidget::toggle_posy_vertices);
  connect(ui->cbShowConsensusGraph, &QCheckBox::toggled,
          ui->animeshGLWidget, &AnimeshGLWidget::toggle_consensus_graph);
  connect(ui->cbShowVertices, &QCheckBox::toggled,
          ui->animeshGLWidget, &AnimeshGLWidget::toggle_vertices);
  connect(ui->cbShowSurface, &QCheckBox::toggled,
          ui->animeshGLWidget, &AnimeshGLWidget::toggle_surface);

  connect(ui->btnSolveRoSy, &QPushButton::clicked, this, &AnimeshWindow::start_solving_rosy);
  connect(ui->btnSolvePoSy, &QPushButton::clicked, this, &AnimeshWindow::start_solving_posy);
  connect(ui->slScale, &QSlider::valueChanged, this, &AnimeshWindow::change_scale);
  connect(ui->btnReset, &QPushButton::clicked, this, &AnimeshWindow::reset_graph);
  connect(ui->btnExportMesh, &QPushButton::clicked, this, &AnimeshWindow::export_mesh);
  connect(ui->btnCollapse, &QPushButton::clicked, this, &AnimeshWindow::collapse_consensus);

  set_ui_for_initialised();

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
  float total = 0.0;
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
AnimeshWindow::collapse_consensus() {
  collapse(m_consensus_graph);
  update();
}

void
AnimeshWindow::set_graph(SurfelGraphPtr &graph) {
  m_graph = graph;
  auto rng = ((AnimeshApp *) QCoreApplication::instance())->random_engine();
  m_multi_res_graph = std::make_shared<MultiResolutionSurfelGraph>(graph, rng);
  m_multi_res_graph->generate_levels(6);
  m_field_optimiser->set_graph(m_multi_res_graph);
  m_consensus_graph = nullptr;
  m_surface_faces.clear();
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
  set_ui_for_graph_loaded();
}

void AnimeshWindow::start_solving_rosy() {
  start_solving(FieldOptimiser::ROSY);
}
void AnimeshWindow::start_solving_posy() {
  start_solving(FieldOptimiser::POSY);
}

void AnimeshWindow::start_solving(FieldOptimiser::SolveMode mode) {
  if (m_graph == nullptr) {
    return;
  }
  if (m_field_optimiser == nullptr) {
    return;
  }
  set_ui_for_solving();
  m_field_optimiser->set_mode(mode);
  QtConcurrent::run([&]() {
    while (!m_field_optimiser->optimise_once()) {
      ui->animeshGLWidget->update();
    }
    if( m_field_optimiser->mode() == FieldOptimiser::ROSY ) {
      set_ui_for_rosy_solved();
    } else {
      set_ui_for_posy_solved();
    }
  });
}

void
AnimeshWindow::export_mesh() {
  if (m_multi_res_graph == nullptr) {
    return;
  }
  m_consensus_graph = build_consensus_graph((*m_multi_res_graph)[0], 0, 1.0f);
  set_ui_for_export();
}

void AnimeshWindow::reset_graph() {
  set_ui_for_graph_loaded();
}

void
AnimeshWindow::generate_surface() {
  using namespace std;
  if (m_graph == nullptr) {
    return;
  }

  /*
   * For each node, find neighbours.
   * Generate a sequence of triangles making sure that the orientation is correct and they are closed
   * Add a canonical tuple of IDs for the triangle to a set to track them (and prevent generating again)
   * add the triangle to the surface.
   */
  Eigen::Vector3f v, t, n;
  set<vector<string>> known_triangles;

  for (const auto &node: m_graph->nodes()) {
    node->data()->get_vertex_tangent_normal_for_frame(0, v, t, n);
    std::vector<Eigen::Vector3f> neighbour_locations;
    const auto neighbours = get_node_neighbours_in_frame(m_graph, node, 0);
    // Not enough to triangulate
    if (neighbours.size() < 2) {
      continue;
    }
    for (int i = 0; i < neighbours.size() - 1; ++i) {
      const auto &n1 = neighbours[i];

      for (int j = i + 1; j < neighbours.size(); ++j) {
        const auto &n2 = neighbours[j];

        if (m_graph->has_edge(n1, n2)) {
          const auto &s0 = node->data()->id();
          const auto &s1 = n1->data()->id();
          const auto &s2 = n2->data()->id();
          vector<string> test{s0, s1, s2};
          sort(test.begin(), test.end());
          if (known_triangles.count(test) == 0) {
            Eigen::Vector3f vn1, tn1, nn1, vn2, tn2, nn2;
            n1->data()->get_vertex_tangent_normal_for_frame(0, vn1, tn1, nn1);
            n2->data()->get_vertex_tangent_normal_for_frame(0, vn2, tn2, nn2);

            // Order the vertices correctly. We want normal of the face to align with normal of the node
            m_surface_faces.emplace_back(v.x());
            m_surface_faces.emplace_back(v.y());
            m_surface_faces.emplace_back(v.z());
            m_surface_faces.emplace_back(n.x());
            m_surface_faces.emplace_back(n.y());
            m_surface_faces.emplace_back(n.z());
            const auto fn = ((vn1 - v).cross(vn2 - vn1));
            if (fn.dot(n) > 0) {
              m_surface_faces.emplace_back(vn1.x());
              m_surface_faces.emplace_back(vn1.y());
              m_surface_faces.emplace_back(vn1.z());
              m_surface_faces.emplace_back(nn1.x());
              m_surface_faces.emplace_back(nn1.y());
              m_surface_faces.emplace_back(nn1.z());
              m_surface_faces.emplace_back(vn2.x());
              m_surface_faces.emplace_back(vn2.y());
              m_surface_faces.emplace_back(vn2.z());
              m_surface_faces.emplace_back(nn2.x());
              m_surface_faces.emplace_back(nn2.y());
              m_surface_faces.emplace_back(nn2.z());
            } else {
              m_surface_faces.emplace_back(vn2.x());
              m_surface_faces.emplace_back(vn2.y());
              m_surface_faces.emplace_back(vn2.z());
              m_surface_faces.emplace_back(nn2.x());
              m_surface_faces.emplace_back(nn2.y());
              m_surface_faces.emplace_back(nn2.z());
              m_surface_faces.emplace_back(vn1.x());
              m_surface_faces.emplace_back(vn1.y());
              m_surface_faces.emplace_back(vn1.z());
              m_surface_faces.emplace_back(nn1.x());
              m_surface_faces.emplace_back(nn1.y());
              m_surface_faces.emplace_back(nn1.z());
            }
            // Emplace the normal

            known_triangles.emplace(test);
          }
        }
      }
    }
  }
}

void AnimeshWindow::set_ui_for_initialised() {
  ui->cbShowVertices->setEnabled(false);
  ui->cbShowSurface->setEnabled(false);
  ui->cbShowNormals->setEnabled(false);
  ui->cbShowTangents->setEnabled(false);
  ui->cbShowMainTangents->setEnabled(false);
  ui->cbShowPoSy->setEnabled(false);
  ui->cbShowConsensusGraph->setEnabled(false);

  ui->btnReset->setEnabled(false);
  ui->btnSolveRoSy->setEnabled(false);
  ui->btnSolvePoSy->setEnabled(false);
  ui->btnExportMesh->setEnabled(false);
}

void AnimeshWindow::set_ui_for_graph_loaded() {
  ui->cbShowVertices->setEnabled(true);
  ui->cbShowVertices->toggle();
  ui->cbShowSurface->setEnabled(true);

  ui->cbShowNormals->setEnabled(true);
  ui->cbShowTangents->setEnabled(true);
  ui->cbShowMainTangents->setEnabled(true);
  ui->cbShowPoSy->setEnabled(true);
  ui->cbShowConsensusGraph->setEnabled(false);

  ui->btnReset->setEnabled(false);
  ui->btnSolveRoSy->setEnabled(true);
  ui->btnSolvePoSy->setEnabled(false);
  ui->btnExportMesh->setEnabled(false);

}

void AnimeshWindow::set_ui_for_solving() {
  ui->cbShowVertices->setEnabled(false);
  ui->cbShowSurface->setEnabled(false);
  ui->cbShowNormals->setEnabled(false);
  ui->cbShowTangents->setEnabled(false);
  ui->cbShowMainTangents->setEnabled(false);
  ui->cbShowPoSy->setEnabled(false);
  ui->cbShowConsensusGraph->setEnabled(false);

  ui->btnReset->setEnabled(false);
  ui->btnSolveRoSy->setEnabled(false);
  ui->btnSolvePoSy->setEnabled(false);
  ui->btnExportMesh->setEnabled(false);
}

void AnimeshWindow::set_ui_for_rosy_solved() {
  ui->cbShowVertices->setEnabled(true);
  ui->cbShowSurface->setEnabled(true);

  ui->cbShowNormals->setEnabled(true);
  ui->cbShowTangents->setEnabled(true);
  ui->cbShowMainTangents->setEnabled(true);
  ui->cbShowPoSy->setEnabled(true);
  ui->cbShowConsensusGraph->setEnabled(false);

  ui->btnReset->setEnabled(true);
  ui->btnSolveRoSy->setEnabled(false);
  ui->btnSolvePoSy->setEnabled(true);
  ui->btnExportMesh->setEnabled(true);
}

void AnimeshWindow::set_ui_for_posy_solved() {
  ui->cbShowVertices->setEnabled(true);
  ui->cbShowSurface->setEnabled(true);

  ui->cbShowNormals->setEnabled(true);
  ui->cbShowTangents->setEnabled(true);
  ui->cbShowMainTangents->setEnabled(true);
  ui->cbShowPoSy->setEnabled(true);
  ui->cbShowConsensusGraph->setEnabled(false);

  ui->btnReset->setEnabled(true);
  ui->btnSolveRoSy->setEnabled(true);
  ui->btnSolvePoSy->setEnabled(false);
  ui->btnExportMesh->setEnabled(true);
}

void AnimeshWindow::set_ui_for_export() {
  ui->cbShowVertices->setEnabled(true);
  ui->cbShowSurface->setEnabled(true);

  ui->cbShowNormals->setEnabled(true);
  ui->cbShowTangents->setEnabled(true);
  ui->cbShowMainTangents->setEnabled(true);
  ui->cbShowPoSy->setEnabled(true);
  ui->cbShowConsensusGraph->setEnabled(true);

  ui->btnReset->setEnabled(true);
  ui->btnSolveRoSy->setEnabled(false);
  ui->btnSolvePoSy->setEnabled(false);
  ui->btnExportMesh->setEnabled(false);
}
