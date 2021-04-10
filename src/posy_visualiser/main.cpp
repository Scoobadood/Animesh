#include "posy_visualiser_window.h"

#include <QApplication>
#include <Surfel/Surfel_IO.h>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
//    const auto graph = load_surfel_graph_from_file(argv[1]);

    posy_visualiser_window w;
    w.show();
    return a.exec();
}
