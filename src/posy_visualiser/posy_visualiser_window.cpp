#include "posy_visualiser_window.h"
#include "ui_posy_visualiser_window.h"
#include <Surfel/SurfelGraph.h>
#include <Surfel/Surfel_IO.h>
#include <QFileDialog>
#include <utility>
#include "posy_surfel_graph_geometry_extractor.h"

posy_visualiser_window::posy_visualiser_window(QWidget *parent):
        QMainWindow(parent)
        , ui(new Ui::posy_visualiser_window)
{
    ui->setupUi(this);
//    ui->statusbar->addPermanentWidget(ui->rosyStatusBar);
    m_geometryExtractor = new posy_surfel_graph_geometry_extractor();

    connect(ui->menuFile, &QMenu::triggered,
            this, &posy_visualiser_window::fileOpenAction);
    connect(ui->frameSelector, &QSlider::valueChanged,
            this, &posy_visualiser_window::frameChanged);
}

posy_visualiser_window::~posy_visualiser_window() {
    delete m_geometryExtractor;
    delete ui;
}


void
posy_visualiser_window::extract_geometry() {
    std::vector<float> positions;
    std::vector<float> quads;
    std::vector<float> normals;
    std::vector<float> uvs;
    m_geometryExtractor->extract_geometry(m_graph_ptr, positions, quads, normals, uvs);
    ui->posyGLWidget->setPoSyData(positions, quads, normals, uvs);
}

void
posy_visualiser_window::set_graph(SurfelGraphPtr graph_ptr) {
    m_graph_ptr = std::move(graph_ptr);
    extract_geometry();
}

void posy_visualiser_window::fileOpenAction() {
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Open Graph"), "",
                                                    tr("Surfel Graph Files (*.bin);;All Files (*)"));
    const auto graphPtr = load_surfel_graph_from_file(fileName.toStdString());
    set_graph(graphPtr);
}

void posy_visualiser_window::frameChanged(int value) {
    m_geometryExtractor->set_frame(value);
    extract_geometry();
}
