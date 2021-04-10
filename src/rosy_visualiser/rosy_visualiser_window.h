#ifndef ROSY_VISUALISER_WINDOW_H
#define ROSY_VISUALISER_WINDOW_H

#include <QMainWindow>
#include <Surfel/SurfelGraph.h>
#include "surfel_graph_geometry_extractor.h"

QT_BEGIN_NAMESPACE
namespace Ui { class rosy_visualiser_window; }
QT_END_NAMESPACE

class rosy_visualiser_window : public QMainWindow
{
    Q_OBJECT

public:
    rosy_visualiser_window(QWidget *parent = nullptr);
    ~rosy_visualiser_window();

    void set_graph(SurfelGraphPtr graph_ptr);

private slots:
    void on_actionOpen_triggered();
    void on_cbNormals_toggled(bool checked);

    void on_cbMainTangent_toggled(bool checked);

    void on_cbOtherTangents_toggled(bool checked);

    void on_slFov_valueChanged(int value);

    void on_slFar_valueChanged(int value);

    void on_frameSelector_valueChanged(int value);

private:
    Ui::rosy_visualiser_window *ui;
    SurfelGraphPtr m_graph_ptr;
    surfel_graph_geometry_extractor * m_geometryExtractor;
    void extract_geometry();

};
#endif // ROSY_VISUALISER_WINDOW_H
