//
// Created by Dave Durbin on 26/3/20.
//

#include <iostream>

#include <nanogui/nanogui.h>
#include "AnimeshApplication.h"
#include "spdlog/cfg/env.h"

int main(int argc, char ** argv) {
    using namespace std;
    spdlog::cfg::load_env_levels();
    try {
        nanogui::init();
        /* scoped variables */
        {
            nanogui::ref <AnimeshApplication> app = new AnimeshApplication(argc, argv);
            app->drawAll();
            app->setVisible(true);
            nanogui::mainloop();
        }
        nanogui::shutdown();
    } catch (const std::runtime_error &e) {
        string error_msg = string("Caught a fatal error: ") + string(e.what());
        cerr << error_msg << endl;
        return -1;
    }
    return 0;
}
