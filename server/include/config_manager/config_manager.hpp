/// GPLv3 LICENSE, Copyright (©) 2025, Maksim Shchavelev <maksimshchavelev@gmail.com>
/// See LICENSE for details

/**
 * @file config_manager/config_manager.hpp
 * @brief Global singleton configuration manager for application and module
 * configuration management
 */

#pragma once

#include <expected>
#include <json/json.h>

namespace smu_server {

/**
 * @brief Global singleton configuration manager for application and module
 * configuration management
 */
class ConfigManager {
  public:
    /**
     * @brief Get instance of ConfigManager
     * @return Lvalue reference to `ConfigManager` instance
     */
    static ConfigManager& instance();




    /**
     * @brief Get *reference* to Json representation of server config
     * @return *Lvalue reference* to `Json::Value` with server config
     * @note Reads the configuration file on the first call
     * @throw `std::runtime` config when error
     */
    Json::Value& get_server_config();




    /**
     * @brief Saves server config.
     * @return `std::expected` with void if success, otherwise `std::string` with error
     */
    std::expected<void, std::string> save_server_config() const noexcept;




    /**
     * @brief Reads config for specified module
     * @param module_name Name of module
     * @return filled `Json::Value` if success, empty `Json::Value` if error
     */
    Json::Value get_module_config(std::string_view module_name) const noexcept;




    /**
     * @brief Saves config of specified module
     * @param module_name Name of module
     * @param config Config of module
     * @return `std::expected` with void if success, otherwise `std::string` with error
     */
    std::expected<void, std::string> save_module_config(std::string_view   module_name,
                                                        const Json::Value& config) const;




  private:
    ConfigManager() = default;
    ~ConfigManager() = default;




    /**
     * @brief Reads config from file
     * @param path Path to file with config
     * @return `std::expected` with `Json::Value` if success and `std::string` if error
     * @private
     */
    std::expected<Json::Value, std::string> read_config(std::string_view path) const noexcept;




    /**
     * @brief Saves config to file
     * @param path Path to file with config
     * @param config Json config
     * @return `std::expected` with void if success, otherwise `std::string` with error
     */
    std::expected<void, std::string> save_config(std::string_view   path,
                                                 const Json::Value& config) const noexcept;


    Json::Value m_server_config{};
};

} // namespace smu_server
