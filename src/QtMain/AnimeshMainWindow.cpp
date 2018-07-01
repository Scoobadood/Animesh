#include <QtWidgets>

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

void AnimeshMainWindow::on_actionOpen_triggered()
{
    // Notify the app to load the file
    QString fileName = QFileDialog::getOpenFileName(this);
    if (!fileName.isEmpty()) {
        loadFile(fileName);
    }
}

void AnimeshMainWindow::loadFile( QString fileName ) {
    setCurrentFile( fileName );
 statusBar()->showMessage(tr("File loaded"), 2000);
}

void AnimeshMainWindow::setCurrentFile(const QString &fileName)
{
    QString shownName = fileName;
    if (fileName.isEmpty()) {
        shownName = "untitled.txt";
    }
    setWindowFilePath(shownName);
}
