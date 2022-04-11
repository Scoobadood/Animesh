#pragma once

#include <QMainWindow>
#include <Surfel/SurfelGraph.h>
#include <ArcBall/TrackBall.h>
#include <Tools/FieldOptimiser.h>
#include <Quad/Quad.h>

QT_BEGIN_NAMESPACE
namespace Ui { class AnimeshWindow; }
QT_END_NAMESPACE

class AnimeshWindow : public QMainWindow {
 Q_OBJECT

 public:
  explicit AnimeshWindow(QWidget *parent = nullptr);
  ~AnimeshWindow() override;

  void set_graph(SurfelGraphPtr &graph);
  inline const SurfelGraphPtr &graph() const {
    return m_graph;
  }
  inline const QuadGraphPtr &consensus_graph() const {
    return m_consensus_graph;
  }

  void start_solving();

 private:
  void fileOpenAction();
  void reset_scale_factor();
  void change_scale(int value);
  void export_mesh();
  void reset_graph();

  Ui::AnimeshWindow *ui;

  SurfelGraphPtr m_graph;
  std::shared_ptr<Trackball> m_arc_ball;
  std::unique_ptr<FieldOptimiser> m_field_optimiser;
  std::shared_ptr<MultiResolutionSurfelGraph> m_multi_res_graph;
  QuadGraphPtr m_consensus_graph;
  float m_scale_factor;
};
