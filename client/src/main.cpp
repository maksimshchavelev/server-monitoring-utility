/// GPLv3 LICENSE, Copyright (©) 2025, Maksim Shchavelev <maksimshchavelev@gmail.com>
/// See LICENSE for details

#include "modules/settings/settings.hpp"

#include <stdexcept>
#include <iostream>

int main(int argc, char** argv) {

    std::optional<smu::Settings> settings;

    try {
        settings = smu::Settings(argc, argv);
    } catch (std::runtime_error& e) {
        std::cout << e.what() << std::endl;
        return -1;
    }

    if(settings->should_exit()) {
        exit(0);
    }

    // Todo:: run application

    return 0;
}
