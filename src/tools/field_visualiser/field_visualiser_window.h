#pragma once

#include <QMainWindow>
#include <Surfel/SurfelGraph.h>
#include <Properties/Properties.h>
#include <ArcBall/ArcBall.h>
#include <QTimer>

#include "posy/posy_surfel_graph_geometry_extractor.h"
#include "rosy/rosy_surfel_graph_geometry_extractor.h"
#include "quad/quad_geometry_extractor.h"

//#include "../../libQuad/include/Quad/Quad.h"

QT_BEGIN_NAMESPACE
namespace Ui { class field_visualiser_window; }
QT_END_NAMESPACE

class field_visualiser_window : public QMainWindow {
 Q_OBJECT

 public:
  rosy_surfel_graph_geometry_extractor *m_rosy_geometry_extractor;

  field_visualiser_window(Properties properties, std::default_random_engine &rng, QWidget *parent = nullptr);

  ~field_visualiser_window() override;

  void set_graph(SurfelGraphPtr graph_ptr);

 private slots:

  void fileOpenAction();

  void frameChanged(int value);

  void quad_vertex_selected(int i);

 private:
  std::shared_ptr<AbstractArcBall> m_arc_ball;
  QTimer *m_timer;
  std::map<std::pair<std::string, std::string>, std::shared_ptr<SurfelGraphEdge>> m_edge_from_node_names;
  std::default_random_engine &m_random_engine;
  Ui::field_visualiser_window *ui;
  SurfelGraphNodePtr m_selected_node;
  bool m_selected_node_is_visible;
  SurfelGraphPtr m_graph_ptr;
  Properties m_properties;
  posy_surfel_graph_geometry_extractor *m_posy_geometry_extractor;
  quad_geometry_extractor *m_quad_geometry_extractor;
  void extract_geometry(bool rebuild_edge_graph = true);
};
