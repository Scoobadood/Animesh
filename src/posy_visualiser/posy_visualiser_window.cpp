#include "posy_visualiser_window.h"
#include "./ui_posy_visualiser_window.h"

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

