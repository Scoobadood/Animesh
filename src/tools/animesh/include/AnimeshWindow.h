#pragma once

#include <QMainWindow>
#include <Surfel/SurfelGraph.h>
#include <ArcBall/TrackBall.h>
#include <Tools/FieldOptimiser.h>

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

  void start_solving();

 private:
  void fileOpenAction();
  void reset_scale_factor();
  void change_scale(int value);

  Ui::AnimeshWindow *ui;

  SurfelGraphPtr m_graph;
  std::shared_ptr<Trackball> m_arc_ball;
  std::unique_ptr<FieldOptimiser> m_field_optimiser;
  std::shared_ptr<MultiResolutionSurfelGraph> m_multi_res_graph;
  float m_scale_factor;
};
