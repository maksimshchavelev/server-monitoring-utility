/// GPLv3 LICENSE, Copyright (©) 2025, Maksim Shchavelev <maksimshchavelev@gmail.com>
/// See LICENSE for details

/**
 * @file config_manager/config_manager.cpp
 * @brief Global singleton configuration manager for application and module
 * configuration management
 */

#include "config_io/config_io.hpp"
#include "compile-time_config.hpp"
#include <filesystem>
#include <fstream>

namespace smu_server {

// Public method
Config_IO& smu_server::Config_IO::instance() {
    static Config_IO manager;
    return manager;
}




// Public method
smu_server::Config Config_IO::get_server_config() {
    auto res = read_config(CONFIG_PATH);
    if (res.has_value()) {
        // If success
        return std::move(res.value());
    } else {
        // If error
        throw std::runtime_error(res.error());
    }
}




// Public method
std::expected<void, std::string> Config_IO::save_server_config(
    const Config& config) const noexcept {
    return save_config(CONFIG_PATH, config);
}




// Public method
Config Config_IO::get_module_config(const std::string_view module_name) const noexcept {
    if (auto config = read_config(
            std::format("{}/{}/{}.json", MODULES_CONFIGS_DIR, module_name, module_name));
        config.has_value()) {
        return config.value();
    }
    return Config();
}




// Public method
std::expected<void, std::string> Config_IO::save_module_config(const std::string_view module_name,
                                                               const Config& config) const {
    return save_config(std::format("{}/{}/{}.json", MODULES_CONFIGS_DIR, module_name, module_name),
                       config);
}




// Private method
std::expected<Config, std::string> Config_IO::read_config(
    const std::string_view path) const noexcept {

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

    return Config(config);
}




// Private method
std::expected<void, std::string> Config_IO::save_config(const std::string_view path,
                                                        const Config& config) const noexcept {

    // Create directories
    std::filesystem::create_directories(std::filesystem::path(path).parent_path());

    std::ofstream file;
    file.open(path.data());

    if (file.is_open()) {
        // If success

        config.get([&](const Json::Value& value) { file << value.toStyledString(); });

        file.close();
        return {};
    } else {
        // If error
        return std::unexpected(std::format("Error opening file with path {}", path));
    }
}




} // namespace smu_server
