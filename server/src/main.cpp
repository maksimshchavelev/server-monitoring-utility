/// GPLv3 LICENSE, Copyright (©) 2025, Maksim Shchavelev <maksimshchavelev@gmail.com>
/// See LICENSE for details

#include "core/core.hpp"
#include "version.hpp"
#include <cxxopts.hpp>
#include <format>
#include <iostream>

int main(int argc, char** argv) {
    cxxopts::Options options("smu-server", "Server part included in server-monitoring-utility");

    options.add_options()("version", "Show smu-server version")("h,help", "Show help information");

    cxxopts::ParseResult result;

    try {
        result = options.parse(argc, argv);
    } catch (std::exception& e) {
        std::cout << "Argument parsing error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    if (result.contains("version")) {
        std::cout << std::format("smu-server version is {}.{}.{}",
                                 PROJECT_VERSION_MAJOR,
                                 PROJECT_VERSION_MINOR,
                                 PROJECT_VERSION_PATCH)
                  << std::endl;
    } else if (result.contains("help") || result.contains("h")) {
        std::cout << options.help();
    }


    // Running application
    smu_server::Application::instance().run();


    return EXIT_SUCCESS;
}
