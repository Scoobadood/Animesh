#include "quad_visualiser_window.h"

#include <QApplication>
#include <Properties/Properties.h>
#include <spdlog/cfg/env.h>

int main(int argc, char *argv[])
{
    spdlog::cfg::load_env_levels();

    QApplication a(argc, argv);

    std::string property_file_name = (argc == 2) ? argv[1] : "animesh.properties";
    Properties properties{property_file_name};

    quad_visualiser_window w(properties);
    w.show();
    return a.exec();
}
