/// GPLv3 LICENSE, Copyright (©) 2025, Maksim Shchavelev <maksimshchavelev@gmail.com>
/// See LICENSE for details

/**
 * @file config_manager/config_manager.hpp
 * @brief A global singleton class that helps read and write configurations in files.
 * configuration management
 */

#pragma once

#include "core/internals/config.hpp"
#include <expected>
#include <json/json.h>

namespace smu_server {

/**
 * @brief A global singleton class that helps read and write configurations in files.
 * @details Can be used in conjunction with `Config`
 * @see Config
 */
class Config_IO {
  public:
    /**
     * @brief Get instance of Config_IO
     * @return Lvalue reference to `Config_IO` instance
     */
    static Config_IO& instance();




    /**
     * @brief Get `smu_server::Config` representation of server config
     * @return `smu_server::Config` with server config
     * @throw `std::runtime` config when error
     */
    Config get_server_config();




    /**
     * @brief Saves server config.
     * @param config Server config
     * @return `std::expected` with void if success, otherwise `std::string` with error
     */
    std::expected<void, std::string> save_server_config(const Config& config) const noexcept;




    /**
     * @brief Reads config for specified module
     * @param module_name Name of module
     * @return filled `smu_server::Config` if success, empty `smu_server::Config` if error
     */
    Config get_module_config(std::string_view module_name) const noexcept;




    /**
     * @brief Saves config of specified module
     * @param module_name Name of module
     * @param config Config of module
     * @return `std::expected` with void if success, otherwise `std::string` with error
     */
    std::expected<void, std::string> save_module_config(std::string_view module_name,
                                                        const Config&    config) const;




  private:
    Config_IO() = default;
    ~Config_IO() = default;




    /**
     * @brief Reads config from file
     * @param path Path to file with config
     * @return `std::expected` with `smu_server::Config` if success and `std::string` if error
     * @private
     */
    std::expected<Config, std::string> read_config(std::string_view path) const noexcept;




    /**
     * @brief Saves config to file
     * @param path Path to file with config
     * @param config Json config
     * @return `std::expected` with void if success, otherwise `std::string` with error
     */
    std::expected<void, std::string> save_config(std::string_view path,
                                                 const Config&    config) const noexcept;
};

} // namespace smu_server
