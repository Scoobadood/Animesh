#include "rosy_visualiser_window.h"
#include "ui_rosy_visualiser_window.h"
#include <Surfel/SurfelGraph.h>
#include <Surfel/Surfel_IO.h>
#include <QFileDialog>
#include "surfel_graph_geometry_extractor.h"

rosy_visualiser_window::rosy_visualiser_window(QWidget *parent)
        : QMainWindow(parent), ui(new Ui::rosy_visualiser_window) {
    ui->setupUi(this);
    ui->statusbar->addPermanentWidget(ui->rosyStatusBar);
    m_geometryExtractor = new surfel_graph_geometry_extractor();
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
    m_graph_ptr = graph_ptr;
    extract_geometry();
}

void rosy_visualiser_window::on_actionOpen_triggered() {
    // Show file dialog
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Open Graph"), "",
                                                    tr("Surfel Graph Files (*.bin);;All Files (*)"));
    const auto graphPtr = load_surfel_graph_from_file(fileName.toStdString());
    set_graph(graphPtr);
}

void rosy_visualiser_window::on_cbNormals_toggled(bool checked) {
    ui->rosyGLWidget->renderNormals(checked);
}

void rosy_visualiser_window::on_cbMainTangent_toggled(bool checked) {
    ui->rosyGLWidget->renderMainTangents(checked);
}

void rosy_visualiser_window::on_cbOtherTangents_toggled(bool checked) {
    ui->rosyGLWidget->renderOtherTangents(checked);
}

void rosy_visualiser_window::on_slFov_valueChanged(int value) {
    ui->rosyGLWidget->setFov((float)value);
}

void rosy_visualiser_window::on_slFar_valueChanged(int value) {
    ui->rosyGLWidget->setZFar((float)value);
}

void rosy_visualiser_window::on_frameSelector_valueChanged(int value) {
    m_geometryExtractor->set_frame(value);
    extract_geometry();
}
