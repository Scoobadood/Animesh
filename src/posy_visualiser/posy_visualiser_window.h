#ifndef POSY_VISUALISER_WINDOW_H
#define POSY_VISUALISER_WINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class posy_visualiser_window; }
QT_END_NAMESPACE

class posy_visualiser_window : public QMainWindow
{
    Q_OBJECT

public:
    posy_visualiser_window(QWidget *parent = nullptr);
    ~posy_visualiser_window();

private:
    Ui::posy_visualiser_window *ui;
};
#endif // POSY_VISUALISER_WINDOW_H
