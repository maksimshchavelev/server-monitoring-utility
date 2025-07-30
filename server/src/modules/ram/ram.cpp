/// GPLv3 LICENSE, Copyright (©) 2025, Maksim Shchavelev <maksimshchavelev@gmail.com>
/// See LICENSE for details

/**
 * @file modules/ram/ram.cpp
 * @brief File with RAM module class
 */


#include "modules/ram/ram.hpp"
#include <sys/sysinfo.h>


namespace smu_server {




// Public method
RAM::RAM(const Config& config) : IModule(config) {
    // Create new configuration
    if (m_configuration.empty()) {
        m_configuration.set("enabled", true);
        m_configuration.set("poll_ratio", 1);
    }

    // Check if the `enabled` field exists. If not, log the error and throw an exception to abort
    // registration.
    if (auto enabled = m_configuration.get<bool>("enabled"); enabled.has_value()) {
        m_enabled = enabled.value();
    } else {
        log(LogType::ERROR, "The 'enabled' field is missing. Can't continue");
        throw std::runtime_error("The 'enabled' field is missing");
    }

    // Check if the `poll_ratio` field exists. If not, log the error and throw an exception to abort
    // registration.
    if (auto poll_ratio = m_configuration.get<unsigned int>("poll_ratio"); poll_ratio.has_value()) {
        m_poll_ratio = poll_ratio.value();
    } else {
        log(LogType::ERROR, "The 'poll_ratio' field is missing. Can't continue");
        throw std::runtime_error("The 'poll_ratio' field is missing");
    }
}




// Public method
std::optional<Json::Value> RAM::get_data() {
    struct sysinfo info;

    if (sysinfo(&info) == -1) { // error
        return std::nullopt;
    } // else


    auto root = make_root_node(
        make_container_node(
            "RAM info",
            make_value_node("Total RAM", info.totalram / 1024 / 1024, "MB"),
            make_value_node("Used RAM", (info.totalram - info.freeram) / 1024 / 1024, "MB"),
            make_value_node(
                "Used RAM (%)", (info.totalram - info.freeram) * 100 / info.totalram, "%")),

        make_container_node(
            "SWAP info",
            make_value_node("Total SWAP", info.totalswap / 1024 / 1024, "MB"),
            make_value_node("SWAP usage", (info.totalswap - info.freeswap) / 1024 / 1024, "MB"),
            make_value_node(
                "SWAP usage (%)", (info.totalswap - info.freeswap) * 100 / info.totalswap, "%")));

    return root->to_json();
}




// Public method
void RAM::enable() {
    m_configuration.set("enabled", true);
    m_enabled = true;
}




// Public method
void RAM::disable() {
    m_configuration.set("enabled", false);
    m_enabled = false;
}

} // namespace smu_server
