#include "rosy_visualiser_window.h"

#include <QApplication>
#include <Surfel/Surfel_IO.h>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    rosy_visualiser_window w;
    w.show();
    return a.exec();
}
