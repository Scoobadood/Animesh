//
// Created by Dave Durbin on 10/4/2022.
//

#pragma once

#include <random>
#include <QApplication>
#include <Properties/Properties.h>

class AnimeshApp : public QApplication{
 public:
  AnimeshApp(int argc, char **argv);

  inline std::default_random_engine & random_engine() {
    return m_random_engine;
  }

 private:

  std::unique_ptr<Properties> m_properties;
  std::default_random_engine m_random_engine;
};
