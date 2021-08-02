#pragma once

#include <QMainWindow>
#include <Surfel/SurfelGraph.h>
#include <Properties/Properties.h>
#include <ArcBall/ArcBall.h>
#include <QTimer>

#include "posy_surfel_graph_geometry_extractor.h"
#include "rosy_surfel_graph_geometry_extractor.h"
#include "../../libQuad/include/Quad/Quad.h"
#include "quad_geometry_extractor.h"

QT_BEGIN_NAMESPACE
namespace Ui { class field_visualiser_window; }
QT_END_NAMESPACE

class field_visualiser_window : public QMainWindow {
Q_OBJECT

public:
    explicit field_visualiser_window(Properties properties, QWidget *parent = nullptr);

    ~field_visualiser_window() override;

    void set_graph(SurfelGraphPtr graph_ptr);

private slots:

    void fileOpenAction();

    void frameChanged(int value);

private:
    ArcBall * m_arc_ball;
    QTimer * m_timer;

    Ui::field_visualiser_window *ui;
    SurfelGraphPtr m_graph_ptr;
    Properties m_properties;
    posy_surfel_graph_geometry_extractor *m_posy_geometry_extractor;
    rosy_surfel_graph_geometry_extractor *m_rosy_geometry_extractor;
    quad_geometry_extractor * m_quad_geometry_extractor;
    void extract_geometry();
};
