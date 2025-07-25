/// GPLv3 LICENSE, Copyright (©) 2025, Maksim Shchavelev <maksimshchavelev@gmail.com>
/// See LICENSE for details

/**
 * @file config_manager/config_manager.hpp
 * @brief A global singleton class that helps read and write configurations in files.
 * configuration management
 */

#pragma once

#include <expected>
#include <json/json.h>

namespace smu_server {

/**
 * @brief A global singleton class that helps read and write configurations in files.
 * @details Can be used in conjunction with `Config`
 * @see Config
 */
class ConfigIO {
  public:
    /**
     * @brief Get instance of ConfigIO
     * @return Lvalue reference to `ConfigIO` instance
     */
    static ConfigIO& instance();




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
    ConfigIO() = default;
    ~ConfigIO() = default;




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
