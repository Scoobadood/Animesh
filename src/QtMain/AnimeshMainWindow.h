#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class AnimeshMainWindow;
}

class AnimeshMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit AnimeshMainWindow(QWidget *parent = 0);
    ~AnimeshMainWindow();

private:
    Ui::AnimeshMainWindow *ui;
};

#endif // MAINWINDOW_H
