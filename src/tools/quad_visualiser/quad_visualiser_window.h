#pragma once

#include <QMainWindow>
#include <Graph/Graph.h>
#include <Eigen/Core>
#include <Properties/Properties.h>
#include <ArcBall/ArcBall.h>
#include <QTimer>

typedef enum {
    EDGE_TYPE_RED = 1,
    EDGE_TYPE_BLU
} EdgeType;


QT_BEGIN_NAMESPACE
namespace Ui { class quad_visualiser_window; }
QT_END_NAMESPACE

class quad_visualiser_window : public QMainWindow {
Q_OBJECT

public:
    explicit quad_visualiser_window(Properties properties, QWidget *parent = nullptr);

    ~quad_visualiser_window() override;

    using GraphPtr = std::shared_ptr<animesh::Graph<Eigen::Vector3f, EdgeType>>;

    void set_graph(const GraphPtr graph_ptr);

private slots:

    void fileOpenAction();

    void frameChanged(int value);

private:
    ArcBall * m_arc_ball;
    QTimer * m_timer;

    Ui::quad_visualiser_window *ui;
    GraphPtr m_graph_ptr;
    Properties m_properties;
    void extract_geometry();
};
