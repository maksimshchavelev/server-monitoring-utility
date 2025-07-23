/// GPLv3 LICENSE, Copyright (©) 2025, Maksim Shchavelev <maksimshchavelev@gmail.com>
/// See LICENSE for details

/**
 * @file core/internals/module.hpp
 * @brief File with IModule class
 */

#pragma once

#include <json/json.h>
#include <optional>

namespace smu_server {

/**
 * @brief The Interface Module class
 * @details Used to obtain information about a specific hardware part of the device
 */
class IModule {
  public:
    /**
     * @brief IModule
     *
     * @param configuration Json::Value with module configuration. See details
     *
     * @details The configuration of the module is stored in a common configuration file,
     * when the module is constructed, it is passed a fragment with the configuration.
     * When the configuration is saved (e.g., when the application exits), it is fetched
     * via the `get_configuration()` method
     *
     * @warning When the program is first run, an empty Json::Value is passed, in which
     * case it must be filled in by yourself.
     *
     * @see get_configuration()
     */
    IModule(const Json::Value& configuration);




    /**
     * @brief Does nothing
     */
    virtual ~IModule() = default;




    /**
     * @brief Method for obtaining module configuration
     * @return Json::Value&
     */
    virtual const Json::Value& get_configuration() const noexcept;




    /**
     * @brief Get module data to send to the client
     * @note Use `make_root_node`, `make_container_node` or `make_value_node` to form correct json.
     * See documentation or [this
     * guide](https://github.com/maksimshchavelev/server-monitoring-utility/blob/master/server/for-developers/own_module.md)
     * @see make_root_node
     * @see make_container_node
     * @see make_value_node
     */
    virtual std::optional<Json::Value> get_data() = 0;




    /**
     * @brief Enable module
     */
    virtual void enable();




    /**
     * @brief Disable module
     */
    virtual void disable();




    /**
     * @brief Is module enabled?
     * @return `true` if module is enabled, otherwise `false`
     */
    virtual bool is_enabled() const;




    /**
     * @brief Get module name
     * @note You do not need to implement this method because the REGISTER_MODULE macro implements
     * it
     * @return `const std::string_view` with module name
     */
    virtual constexpr std::string_view module_name() const = 0;




    /**
     * @brief Get module description
     * @note You do not need to implement this method because the REGISTER_MODULE macro implements
     * it
     * @return `const std::string_view` with module description
     */
    virtual constexpr std::string_view module_description() const = 0;



  protected:
    Json::Value m_configuration;
    bool        m_enabled{false};

    // =============================== LOGGER ===============================

    /**
     * @brief Describes log type. Affects the color of messages
     */
    enum class LogType { INFO, WARNING, ERROR };




    /**
     * @brief Outputs the log
     * @param log_type Log type. Takes the following values:
     * LogType::INFO - white log
     * LogType::INFO - yellow log
     * LogType::ERROR - red log
     * @param message Message to log
     * @note Prints white message if `log_type` is incorrect
     */
    void log(LogType log_type, const std::string_view message);
};

} // end of namespace smu_server
