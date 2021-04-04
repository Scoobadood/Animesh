#ifndef ROSY_VISUALISER_WINDOW_H
#define ROSY_VISUALISER_WINDOW_H

#include <QMainWindow>
#include <Surfel/SurfelGraph.h>

QT_BEGIN_NAMESPACE
namespace Ui { class rosy_visualiser_window; }
QT_END_NAMESPACE

class rosy_visualiser_window : public QMainWindow
{
    Q_OBJECT

public:
    rosy_visualiser_window(QWidget *parent = nullptr);
    ~rosy_visualiser_window();

    void set_graph(SurfelGraphPtr graph_ptr);

private slots:
    void on_actionOpen_triggered();
private:
    Ui::rosy_visualiser_window *ui;
    SurfelGraphPtr m_graph_ptr;
};
#endif // ROSY_VISUALISER_WINDOW_H
