/// GPLv3 LICENSE, Copyright (©) 2025, Maksim Shchavelev <maksimshchavelev@gmail.com>
/// See LICENSE for details

/**
 * @file modules/settings/settings.cpp
 * @brief File with settings to setup when startup
 */

#include "modules/settings/settings.hpp"
#include "compile-time_config.hpp"
#include "version.hpp"
#include <cxxopts.hpp>
#include <iostream>

namespace smu {

// Public constructor
Settings::Settings(int argc, char** argv) : m_port(DEFAULT_PORT) {
    if (auto res = parse(argc, argv); !res.has_value()) {
        throw std::runtime_error(std::format("Error parsing arguments: {}", res.error()));
    }
}




// Public method
bool Settings::should_exit() const noexcept {
    return m_should_exit;
}




// Public method
uint16_t Settings::get_port() const noexcept {
    return m_port;
}




// Public method
const std::string_view Settings::get_ip() const noexcept {
    return m_ip;
}




// Private method
std::expected<void, std::string> Settings::parse(int argc, char** argv) noexcept {
    cxxopts::Options options("smu", "Client part included in server-monitoring-utility");

    options.add_options()("version", "Show smu version");
    options.add_options()("h,help", "Show help information");
    options.add_options()(
        "p,port", "Specify the port to connect to the smu-server", cxxopts::value<uint16_t>());

    // Positional options
    options.add_options("Positional")("ip", "Server IP address", cxxopts::value<std::string>());
    options.parse_positional("ip");

    cxxopts::ParseResult result;

    try {
        result = options.parse(argc, argv);
    } catch (std::exception& e) {
        return std::unexpected(e.what());
    }

    // VERSION
    if (result.contains("version")) {
        std::cout << std::format("Version is {}.{}.{}",
                                 PROJECT_VERSION_MAJOR,
                                 PROJECT_VERSION_MINOR,
                                 PROJECT_VERSION_PATCH)
                  << std::endl;
        m_should_exit = true;
        return {};
    }

    // HELP
    if (result.contains("help")) {
        std::cout << options.help();
        m_should_exit = true;
        return {};
    }

    // PORT
    if (result.contains("port")) {
        // Set port
        m_port = result["port"].as<uint16_t>();
    }

    // IP
    if (result.contains("ip")) {
        m_ip = result["ip"].as<std::string>();
    } else {
        return std::unexpected("IP address is required");
    }


    return {};
}




} // namespace smu
