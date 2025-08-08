/// GPLv3 LICENSE, Copyright (©) 2025, Maksim Shchavelev <maksimshchavelev@gmail.com>
/// See LICENSE for details

#include "modules/application/application.hpp"
#include "modules/settings/settings.hpp"

#include <csignal>
#include <iostream>


smu::Application* app_ptr{nullptr};


void exit_handler(int) {
    app_ptr->exit(std::nullopt);
}


int main(int argc, char** argv) {

    std::optional<smu::Settings> settings;

    try {
        settings = smu::Settings(argc, argv);
    } catch (std::runtime_error& e) {
        std::cout << e.what() << std::endl;
        return -1;
    }

    if (settings->should_exit()) {
        exit(0);
    }

    // Run application
    smu::Application app(*settings);
    app_ptr = &app;

    signal(SIGINT, exit_handler);

    return app.run();
}
