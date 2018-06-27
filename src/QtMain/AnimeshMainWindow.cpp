#include "AnimeshMainWindow.h"
#include "ui_AnimeshMainWindow.h"

AnimeshMainWindow::AnimeshMainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::AnimeshMainWindow)
{
    ui->setupUi(this);
}

AnimeshMainWindow::~AnimeshMainWindow()
{
    delete ui;
}
