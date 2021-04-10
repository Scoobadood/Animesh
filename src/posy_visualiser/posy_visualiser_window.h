#ifndef POSY_VISUALISER_WINDOW_H
#define POSY_VISUALISER_WINDOW_H

#include <QMainWindow>
#include <Surfel/SurfelGraph.h>

QT_BEGIN_NAMESPACE
namespace Ui { class posy_visualiser_window; }
QT_END_NAMESPACE

class posy_visualiser_window : public QMainWindow
{
    Q_OBJECT

public:
    posy_visualiser_window(QWidget *parent = nullptr);
    ~posy_visualiser_window();

    void set_graph(SurfelGraphPtr graph_ptr);

private slots:
    void on_actionOpen_triggered();

private:
    Ui::posy_visualiser_window *ui;
    SurfelGraphPtr m_graph_ptr;
};
#endif // POSY_VISUALISER_WINDOW_H
