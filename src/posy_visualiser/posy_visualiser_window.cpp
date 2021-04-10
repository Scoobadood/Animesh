#include "posy_visualiser_window.h"
#include "./ui_posy_visualiser_window.h"
#include <Surfel/SurfelGraph.h>
#include <Surfel/Surfel_IO.h>
#include <QFileDialog>

posy_visualiser_window::posy_visualiser_window(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::posy_visualiser_window)
{
    ui->setupUi(this);
}

posy_visualiser_window::~posy_visualiser_window()
{
    delete ui;
}

void
posy_visualiser_window::set_graph(SurfelGraphPtr graph_ptr) {
    m_graph_ptr = graph_ptr;
}

void posy_visualiser_window::on_actionOpen_triggered()
{
    // Show file dialog
    QString fileName = QFileDialog::getOpenFileName(this,
            tr("Open Graph"), "",
            tr("Surfel Graph Files (*.bin);;All Files (*)"));
    const auto graph = load_surfel_graph_from_file(fileName.toStdString());
//    ui->openGLWidget->setPoSyData(graph);
}
