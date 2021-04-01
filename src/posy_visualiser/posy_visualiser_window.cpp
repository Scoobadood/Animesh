#include "posy_visualiser_window.h"
#include "./ui_posy_visualiser_window.h"
#include <Surfel/SurfelGraph.h>

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
