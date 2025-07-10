/// GPLv3 LICENSE, Copyright (©) 2025, Maksim Shchavelev <maksimshchavelev@gmail.com>
/// See LICENSE for details

/**
 * @file config_manager/config_manager.cpp
 * @brief Global singleton configuration manager for application and module
 * configuration management
 */

#include "config_manager/config_manager.hpp"
#include "compile-time_config.hpp"
#include <fstream>

namespace smu_server {

// Public method
ConfigManager& smu_server::ConfigManager::instance() {
    static ConfigManager manager;
    return manager;
}




// Public method
Json::Value& ConfigManager::get_server_config() {
    if (m_server_config.empty()) {
        auto res = read_config(CONFIG_PATH);
        if (res.has_value()) {
            // If success
            m_server_config = std::move(res.value());
        } else {
            // If error
            throw std::runtime_error(res.error());
        }
    }

    return m_server_config;
}




// Public method
std::expected<void, std::string> ConfigManager::save_server_config() const noexcept {
    return save_config(CONFIG_PATH, m_server_config);
}




// Public method
Json::Value ConfigManager::get_module_config(std::string_view module_name) const noexcept {
    if (auto config = read_config(std::format("{}/{}.json", MODULES_CONFIGS_DIR, module_name));
        config.has_value()) {
        return config.value();
    }
    return Json::Value();
}




// Public method
std::expected<void, std::string> ConfigManager::save_module_config(
    std::string_view module_name, const Json::Value& config) const {
    return save_config(std::format("{}/{}.json", MODULES_CONFIGS_DIR, module_name), config);
}




// Private method
std::expected<Json::Value, std::string> ConfigManager::read_config(
    std::string_view path) const noexcept {

    std::ifstream json_file;
    json_file.open(path.data());

    if (!json_file.is_open()) {
        return std::unexpected(std::format("error reading config file with path {} (cause: {})",
                                           path,
                                           std::strerror(errno))); // error
    }

    // string representation of json
    std::string raw_json{std::istreambuf_iterator<char>(json_file),
                         std::istreambuf_iterator<char>()};

    // Converting string to json
    const auto     raw_json_length = static_cast<int>(raw_json.length());
    JSONCPP_STRING err;
    Json::Value    config;

    Json::CharReaderBuilder                 builder;
    const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
    if (!reader->parse(raw_json.c_str(), raw_json.c_str() + raw_json_length, &config, &err)) {
        // error
        return std::unexpected(std::format("Error conversion to json: {}", err));
    }

    return config;
}




// Private method
std::expected<void, std::string> ConfigManager::save_config(
    std::string_view path, const Json::Value& config) const noexcept {
    std::ofstream file;
    file.open(path.data());

    if (file.is_open()) {
        // If success
        file << config.toStyledString();
        file.close();
        return {};
    } else {
        // If error
        return std::unexpected(std::format("Error opening file with path {}", path));
    }
}




} // namespace smu_server
