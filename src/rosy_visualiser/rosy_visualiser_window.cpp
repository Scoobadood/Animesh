#include "rosy_visualiser_window.h"
#include "ui_rosy_visualiser_window.h"
#include <Surfel/SurfelGraph.h>
#include <Surfel/Surfel_IO.h>
#include <QFileDialog>
#include <utility>
#include "surfel_graph_geometry_extractor.h"

rosy_visualiser_window::rosy_visualiser_window(QWidget *parent)
        : QMainWindow(parent), ui(new Ui::rosy_visualiser_window) {
    ui->setupUi(this);
    ui->statusbar->addPermanentWidget(ui->rosyStatusBar);
    m_geometryExtractor = new posy_surfel_graph_geometry_extractor();

    connect(ui->cbNormals, &QCheckBox::toggled,
            ui->rosyGLWidget, &rosy_gl_widget::renderNormals);
    connect(ui->cbMainTangent, &QCheckBox::toggled,
            ui->rosyGLWidget, &rosy_gl_widget::renderMainTangents);
    connect(ui->cbOtherTangents, &QCheckBox::toggled,
            ui->rosyGLWidget, &rosy_gl_widget::renderOtherTangents);
    connect(ui->slFar, &QSlider::valueChanged,
            ui->rosyGLWidget, &rosy_gl_widget::setZFar);
    connect(ui->slFov, &QSlider::valueChanged,
            ui->rosyGLWidget, &rosy_gl_widget::setFov);
    connect(ui->menuFile, &QMenu::triggered,
            this, &rosy_visualiser_window::fileOpenAction);
    connect(ui->frameSelector, &QSlider::valueChanged,
            this, &rosy_visualiser_window::frameChanged);
}

rosy_visualiser_window::~rosy_visualiser_window() {
    delete m_geometryExtractor;
    delete ui;
}


void
rosy_visualiser_window::extract_geometry() {
    std::vector<float> positions;
    std::vector<float> normals;
    std::vector<float> tangents;
    float scale_factor;
    m_geometryExtractor->extract_geometry(m_graph_ptr, positions, tangents, normals, scale_factor);
    ui->rosyGLWidget->setRoSyData(positions, normals, tangents, scale_factor);
}

void
rosy_visualiser_window::set_graph(SurfelGraphPtr graph_ptr) {
    m_graph_ptr = std::move(graph_ptr);
    extract_geometry();
}

void rosy_visualiser_window::fileOpenAction() {
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Open Graph"), "",
                                                    tr("Surfel Graph Files (*.bin);;All Files (*)"));
    const auto graphPtr = load_surfel_graph_from_file(fileName.toStdString());
    set_graph(graphPtr);
}

void rosy_visualiser_window::frameChanged(int value) {
    m_geometryExtractor->set_frame(value);
    extract_geometry();
}
