/// GPLv3 LICENSE, Copyright (©) 2025, Maksim Shchavelev <maksimshchavelev@gmail.com>
/// See LICENSE for details

/**
 * @file modules/ram/ram.cpp
 * @brief File with RAM module class
 */

#include "modules/ram/ram.hpp"

namespace smu_server {

// Public method
const Json::Value& RAM::get_configuration() const {
    return m_configuration;
}




// Public method
std::optional<Json::Value> RAM::get_data() {
    if (!is_enabled())
        return std::nullopt;

    Json::Value root;
    root["type"] = "value";
    root["value"] = 4.7;
    root["unit"] = "GB";

    return root;
}

} // namespace smu_server
