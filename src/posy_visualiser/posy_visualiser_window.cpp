#include "posy_visualiser_window.h"
#include "posy_surfel_graph_geometry_extractor.h"
#include "ui_posy_visualiser_window.h"
#include <Surfel/SurfelGraph.h>
#include <Surfel/Surfel_IO.h>
#include <QFileDialog>
#include <utility>

posy_visualiser_window::posy_visualiser_window(Properties properties, QWidget *parent) :
        QMainWindow(parent), ui(new Ui::posy_visualiser_window), m_properties{std::move(properties)} {
    ui->setupUi(this);

    float rho = m_properties.getFloatProperty("rho");

    ui->posyGLWidget->setRho(rho);

    m_geometryExtractor = new posy_surfel_graph_geometry_extractor(rho);

    connect(ui->menuFile, &QMenu::triggered,
            this, &posy_visualiser_window::fileOpenAction);
    connect(ui->frameSelector, &QSlider::valueChanged,
            this, &posy_visualiser_window::frameChanged);
    connect(ui->horizontalSlider, &QSlider::valueChanged,
            [=](int value) {
                float mapped_value = value / 10.0f;
//                ui->posyGLWidget->setSplatSize(newSize);
                m_geometryExtractor->set_splat_scale_factor(0.5f + mapped_value);
                extract_geometry();
            });
    connect(ui->cbShowQuads, &QCheckBox::toggled, ui->posyGLWidget, &posy_gl_widget::render_quads);
    connect(ui->cbShowTextures, &QCheckBox::toggled, ui->posyGLWidget, &posy_gl_widget::render_textures);
    connect(ui->cbShowTriangleFans, &QCheckBox::toggled, ui->posyGLWidget, &posy_gl_widget::render_triangle_fans);
}

posy_visualiser_window::~posy_visualiser_window() {
    delete m_geometryExtractor;
    delete ui;
}


void
posy_visualiser_window::extract_geometry() {
    std::vector<float> positions;
    std::vector<float> quads;
    std::vector<float> triangle_fans;
    std::vector<float> triangle_uvs;
    std::vector<unsigned int> fan_sizes;
    std::vector<float> normals;
    std::vector<float> splat_sizes;
    std::vector<float> uvs;
    m_geometryExtractor->extract_geometry(
            m_graph_ptr,
            positions,
            quads,
            triangle_fans,
            triangle_uvs,
            fan_sizes,
            normals,
            splat_sizes,
            uvs);
    ui->posyGLWidget->setPoSyData(positions, quads,
                                  triangle_fans,
                                  triangle_uvs,
                                  fan_sizes,
                                  normals, splat_sizes, uvs);
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
