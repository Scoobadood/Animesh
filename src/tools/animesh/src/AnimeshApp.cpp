//
// Created by Dave Durbin on 10/4/2022.
//

#include <QCommandLineParser>
#include <memory>
#include <Properties/Properties.h>
#include "AnimeshApp.h"

AnimeshApp::AnimeshApp(int argc, char **argv) //
    : QApplication(argc, argv) {
  QCommandLineParser parser;
  parser.setApplicationDescription(QCoreApplication::applicationName());
  parser.addHelpOption();
  parser.addVersionOption();
  parser.addPositionalArgument("file", "Properties file.");
  parser.process(*this);

  auto property_file_name = parser.positionalArguments().isEmpty()
                            ? "animesh.properties"
                            : parser.positionalArguments().first();

  m_properties = std::make_unique<Properties>(property_file_name.toStdString());
  m_random_engine = std::default_random_engine{123};
};