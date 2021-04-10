#include "rosy_visualiser_window.h"

#include <QSurfaceFormat>
#include <QOpenGLContext>
#include <QApplication>

int
main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    rosy_visualiser_window w;
    w.show();
    return a.exec();
}
