#include "rosy_visualiser_status_bar.h"
#include "ui_rosy_visualiser_status_bar.h"

rosy_visualiser_status_bar::rosy_visualiser_status_bar(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::rosy_visualiser_status_bar)
{
    ui->setupUi(this);
}

rosy_visualiser_status_bar::~rosy_visualiser_status_bar()
{
    delete ui;
}

void rosy_visualiser_status_bar::cameraPositionChanged(float x, float y, float z) {
    ui->lblXValue->setText(QString::number(x));
    ui->lblYValue->setText(QString::number(y));
    ui->lblZValue->setText(QString::number(z));
}

void rosy_visualiser_status_bar::cameraOrientationChanged(float radius, float theta, float phi) {
    ui->lblRadiusValue->setText(QString::number(radius));
    ui->lblThetaValue->setText(QString::number(theta));
    ui->lblPhiValue->setText(QString::number(phi));
}
