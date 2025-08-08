/// GPLv3 LICENSE, Copyright (©) 2025, Maksim Shchavelev <maksimshchavelev@gmail.com>
/// See LICENSE for details

#include "core/core.hpp"

int main(int argc, char** argv) {

    // Running application
    smu_server::Application::instance().run(argc, argv);

    return EXIT_SUCCESS;
}
