#ifndef ROSY_VISUALISER_STATUS_BAR_H
#define ROSY_VISUALISER_STATUS_BAR_H

#include <QWidget>

namespace Ui {
class rosy_visualiser_status_bar;
}

class rosy_visualiser_status_bar : public QWidget
{
    Q_OBJECT

public:
    explicit rosy_visualiser_status_bar(QWidget *parent = nullptr);
    ~rosy_visualiser_status_bar();

private:
    Ui::rosy_visualiser_status_bar *ui;

public slots:
    void cameraPositionChanged(float x, float y, float z);
    void cameraOrientationChanged(float roll, float pitch, float yaw);
};

#endif // ROSY_VISUALISER_STATUS_BAR_H
