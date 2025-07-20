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
RAM::RAM(const Json::Value& config) : IModule(config) {
    // Create new configuration
    if (m_configuration.empty()) {
        m_configuration["enabled"] = true;
    }

    m_enabled = m_configuration["enabled"].asBool();
}




// Public method
const Json::Value& RAM::get_configuration() const {
    return m_configuration;
}




// Public method
std::optional<Json::Value> RAM::get_data() {
    if (!is_enabled())
        return std::nullopt;

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
    m_configuration["enabled"] = true;
}




// Public method
void RAM::disable() {
    m_configuration["enabled"] = false;
}

} // namespace smu_server
