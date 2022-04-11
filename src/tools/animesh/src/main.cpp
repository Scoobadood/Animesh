//
// Created by Dave Durbin on 10/4/2022.
//

#include <QCommandLineParser>
#include <QString>
#include <spdlog/cfg/env.h>
#include <AnimeshWindow.h>
#include <AnimeshApp.h>

int main(int argc, char *argv[]) {
  spdlog::cfg::load_env_levels();

  AnimeshApp app(argc, argv);
  QCoreApplication::setOrganizationName("QtProject");
  QCoreApplication::setApplicationName("Application Example");
  QCoreApplication::setApplicationVersion(QT_VERSION_STR);

  AnimeshWindow animeshWindow;
  animeshWindow.show();
  return AnimeshApp::exec();
}
