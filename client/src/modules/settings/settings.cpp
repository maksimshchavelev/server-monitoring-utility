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
#include <filesystem>
#include <fstream>
#include <iostream>

namespace smu {

// Public constructor
Settings::Settings(int argc, char** argv) : m_port(DEFAULT_PORT) {
    if (auto res = parse(argc, argv); !res.has_value()) {
        throw std::runtime_error(res.error());
    }

    // For Windows. We must manually create a directory with settings and subdirectories. For Linux,
    // this is done by the installation package.
    #if defined(_WIN32)
    std::filesystem::create_directories(CONFIG_ROOT_DIR);
    std::filesystem::create_directories(std::format("{}certs", CONFIG_ROOT_DIR));
    #endif
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

    options.add_options("Positional")("ip", "Server IP address", cxxopts::value<std::string>());
    options.parse_positional({"ip"});
    options.show_positional_help();

    options.add_options()(
        "loadcert",
        "Load a trusted certificate for a specific server with a specific IP address. Usage:\nsmu "
        "--loadcert --cert <path to cert> --trust-ip <server ip>")(
        "cert", "Path to certificate (used only with --loadcert)", cxxopts::value<std::string>())(
        "trust-ip",
        "The IP to associate the certificate with (used only with --loadcert)",
        cxxopts::value<std::string>());


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

    // LOADCERT
    if (result.contains("loadcert")) {
        if (!result.contains("cert") || !result.contains("trust-ip")) {
            return std::unexpected("Wrong --loadcert syntax! Correct syntax is --loadcert --cert "
                                   "<path to cert> --trust-ip <server ip>");
        }

        const auto& path_to_cert = result["cert"].as<std::string>();
        const auto& server_ip = result["trust-ip"].as<std::string>();

        // Copy certificate
        const std::string dest_path = std::format("{}certs/{}.crt", CONFIG_ROOT_DIR, server_ip);
        std::ifstream     source(path_to_cert, std::ios::binary);
        std::ofstream     dest(dest_path, std::ios::binary);

        // Error opening source
        if (!source.is_open()) {
            return std::unexpected(
                std::format("Failed to open {} for reading: {}", path_to_cert, strerror(errno)));
        }

        // Error opening dest
        if (!dest.is_open()) {
            return std::unexpected(
                std::format("Failed to open {} for writing: {}", dest_path, strerror(errno)));
        }

        dest << source.rdbuf();

        dest.close();
        source.close();

        std::cout << "Done!" << std::endl;

        m_should_exit = true; // Do not continue
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
