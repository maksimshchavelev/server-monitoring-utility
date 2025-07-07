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
    enable();
}




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


    Json::Value root;

    // TOTAL RAM
    Json::Value total_ram;
    total_ram["type"] = "value";
    total_ram["value"] = std::to_string(info.totalram / 1024 / 1024);
    total_ram["units"] = "MB";

    // USED RAM
    Json::Value used_ram;
    used_ram["type"] = "value";
    used_ram["value"] = std::to_string((info.totalram - info.freeram) / 1024 / 1024);
    used_ram["units"] = "MB";

    // USED RAM IN PERCENT
    Json::Value used_ram_in_percent;
    used_ram_in_percent["type"] = "value";
    used_ram_in_percent["value"] =
        std::to_string((info.totalram - info.freeram) * 100 / info.totalram);
    used_ram_in_percent["units"] = "%";

    // SWAP TOTAL
    Json::Value swap_total;
    swap_total["type"] = "value";
    swap_total["value"] = std::to_string(info.totalswap / 1024 / 1024);
    swap_total["units"] = "MB";

    // SWAP USAGE
    Json::Value swap_usage;
    swap_usage["type"] = "value";
    swap_usage["value"] = std::to_string((info.totalswap - info.freeswap) / 1024 / 1024);
    swap_usage["units"] = "MB";


    root["RAM size"] = total_ram;
    root["RAM used"] = used_ram;
    root["Used RAM in percent"] = used_ram_in_percent;
    root["SWAP total"] = swap_total;
    root["SWAP usage"] = swap_usage;

    return root;
}

} // namespace smu_server
