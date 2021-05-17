#pragma once

#include <QMainWindow>
#include <Surfel/SurfelGraph.h>
#include "rosy_surfel_graph_geometry_extractor.h"

QT_BEGIN_NAMESPACE
namespace Ui { class rosy_visualiser_window; }
QT_END_NAMESPACE

class rosy_visualiser_window : public QMainWindow
{
    Q_OBJECT

public:
    explicit rosy_visualiser_window(QWidget *parent = nullptr);
    ~rosy_visualiser_window() override;

    void set_graph(SurfelGraphPtr graph_ptr);

private slots:
    void fileOpenAction();
    void frameChanged(int value);

private:
    Ui::rosy_visualiser_window *ui;
    SurfelGraphPtr m_graph_ptr;
    rosy_surfel_graph_geometry_extractor * m_geometryExtractor;
    void extract_geometry();

};
