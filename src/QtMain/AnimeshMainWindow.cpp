#include <QtWidgets>

#include "AnimeshMainWindow.h"
#include "ui_AnimeshMainWindow.h"

#include "vtkActor.h"
#include "vtkRenderer.h"
#include "vtkCylinderSource.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkPolyDataMapper.h"
#include "vtkFloatArray.h"
#include "vtkPointData.h"
#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkProperty.h"
#include "vtkDataArray.h"
#include "vtkLine.h"
#include "vtkUnsignedCharArray.h"
#include "vtkButtonWidget.h"
#include "vtkCoordinate.h"
#include "vtkSphereSource.h"
#include "vtkTexturedButtonRepresentation2D.h"
#include "vtkCallbackCommand.h"
#include "vtkCamera.h"
#include "vtkNamedColors.h"
#include "vtkTextWidget.h"
#include "vtkTextActor.h"
#include "vtkTextProperty.h"
#include "vtkTextRepresentation.h"
#include "vtkInteractorStyleTrackballCamera.h"
#include "vtkAbstractPicker.h"
#include "vtkPointPicker.h"
#include "vtkRendererCollection.h"


AnimeshMainWindow::AnimeshMainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::AnimeshMainWindow)
{
    ui->setupUi(this);

    // Sphere
    vtkSmartPointer<vtkSphereSource> sphereSource = vtkSmartPointer<vtkSphereSource>::New();
    sphereSource->Update();

    vtkSmartPointer<vtkPolyDataMapper> sphereMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    sphereMapper->SetInputConnection(sphereSource->GetOutputPort());

    vtkSmartPointer<vtkActor> sphereActor = vtkSmartPointer<vtkActor>::New();
    sphereActor->SetMapper(sphereMapper);

    // VTK Renderer
    vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
    renderer->AddActor(sphereActor);

    // VTK/Qt wedded
    this->ui->qvtkWidget->GetRenderWindow()->AddRenderer(renderer);

    // Set up action signals and slots
    connect(this->ui->actionExit, SIGNAL(triggered()), this, SLOT(slotExit()));
 
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