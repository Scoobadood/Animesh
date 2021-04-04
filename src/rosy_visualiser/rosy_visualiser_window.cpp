#include "rosy_visualiser_window.h"
#include "./ui_rosy_visualiser_window.h"
#include <Surfel/SurfelGraph.h>
#include <Surfel/Surfel_IO.h>
#include <QFileDialog>

#include "surfel_graph_geometry_extractor.h"

rosy_visualiser_window::rosy_visualiser_window(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::rosy_visualiser_window)
{
    ui->setupUi(this);
    ui->statusbar->addPermanentWidget(ui->rosyStatusBar);
}

rosy_visualiser_window::~rosy_visualiser_window()
{
    delete ui;
}

void
rosy_visualiser_window::set_graph(SurfelGraphPtr graph_ptr) {
    m_graph_ptr = graph_ptr;
}

void rosy_visualiser_window::on_actionOpen_triggered()
{
    // Show file dialog
    QString fileName = QFileDialog::getOpenFileName(this,
            tr("Open Graph"), "",
            tr("Surfel Graph Files (*.bin);;All Files (*)"));
    const auto graph = load_surfel_graph_from_file(fileName.toStdString());
    auto sg = new surfel_graph_geometry_extractor();
    std::vector<float> positions;
    std::vector<float> normals;
    std::vector<float> tangents;
    float scale_factor;
    sg->extract_geometry(graph,positions, tangents, normals, scale_factor);
    ui->rosyGLWidget->setRoSyData(positions, normals, tangents, scale_factor);
}
