#include "quad_visualiser_window.h"
#include "ui_quad_visualiser_window.h"
#include <Surfel/Surfel_IO.h>
#include <Quad/Quad.h>
#include <QFileDialog>
#include <utility>
#include <map>
#include <memory>

quad_visualiser_window::quad_visualiser_window(Properties properties, QWidget *parent) :
        QMainWindow(parent), ui(new Ui::quad_visualiser_window), m_properties{std::move(properties)} {
    ui->setupUi(this);

    m_arc_ball = new ArcBall();
    ui->quadGLWidget->set_arc_ball(m_arc_ball);

    connect(ui->menuFile, &QMenu::triggered,
            this, &quad_visualiser_window::fileOpenAction);
    connect(ui->frameSelector, &QSlider::valueChanged,
            this, &quad_visualiser_window::frameChanged);
    connect(ui->cbBlue, &QCheckBox::toggled,
            ui->quadGLWidget, &quad_gl_widget::showBlueEdges);
    connect(ui->cbRed, &QCheckBox::toggled,
            ui->quadGLWidget, &quad_gl_widget::showRedEdges);

    m_timer = new QTimer(this); //Create a timer
    m_timer->callOnTimeout([=](){
        ui->quadGLWidget->update();
    });
    m_timer->start(10);
}

quad_visualiser_window::~quad_visualiser_window() {
    delete m_arc_ball;
    delete ui;
}


void
quad_visualiser_window::extract_geometry() {
    using namespace std;

    vector<float> vertices;
    vector<pair<unsigned int, unsigned int>> red_edges;
    vector<pair<unsigned int, unsigned int>> blue_edges;

    map<const shared_ptr<animesh::Graph<Eigen::Vector3f, EdgeType>::GraphNode>, unsigned int> node_id_to_vertex_index;
    unsigned int vertex_index = 0;
    for (const auto &node : m_graph_ptr->nodes()) {
        node_id_to_vertex_index.insert({node, vertex_index});
        float x = node->data().x();
        float y = node->data().y();
        float z = node->data().z();
        vertices.emplace_back(x);
        vertices.emplace_back(y);
        vertices.emplace_back(z);
        vertex_index++;
    }
    for (const auto &edge : m_graph_ptr->edges()) {
        unsigned int from = node_id_to_vertex_index.at(edge.from());
        unsigned int to = node_id_to_vertex_index.at(edge.to());

        if (*edge.data() == EDGE_TYPE_RED) {
            red_edges.emplace_back(make_pair(from, to));
        } else {
            blue_edges.emplace_back(make_pair(from, to));
        }
    }
    ui->quadGLWidget->setData(vertices, red_edges, blue_edges);
}

void
quad_visualiser_window::set_graph(const GraphPtr graph_ptr) {
    m_graph_ptr = graph_ptr;
    extract_geometry();
}


void quad_visualiser_window::fileOpenAction() {
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Open Graph"), "",
                                                    tr("Surfel Graph Files (*.bin);;All Files (*)"));
    const auto surfelGraphPtr = load_surfel_graph_from_file(fileName.toStdString());
    GraphPtr graphPtr = build_edge_graph(0, surfelGraphPtr);
    set_graph(graphPtr);
}

void quad_visualiser_window::frameChanged(int value) {
    //TODO: Set frame;
    extract_geometry();
}
