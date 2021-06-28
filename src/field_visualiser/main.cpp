#include "field_visualiser_window.h"

#include <QApplication>
#include <Properties/Properties.h>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    std::string property_file_name = (argc == 2) ? argv[1] : "animesh.properties";
    Properties properties{property_file_name};

    field_visualiser_window w(properties);
    w.show();
    return a.exec();
}
