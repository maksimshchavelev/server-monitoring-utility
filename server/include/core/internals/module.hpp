/// GPLv3 LICENSE, Copyright (©) 2025, Maksim Shchavelev <maksimshchavelev@gmail.com>
/// See LICENSE for details

/**
 * @file core/internals/module.hpp
 * @brief File with IModule class
 */

#pragma once

#include "config.hpp"
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
     * @param configuration `Config` with module configuration. See details
     *
     * @details The configuration of the module is stored in a common configuration file,
     * when the module is constructed, it is passed a fragment with the configuration.
     * When the configuration is saved (e.g., when the application exits), it is fetched
     * via the `get_configuration()` method
     *
     * @warning When the program is first run, an empty `Json::Value` is passed, in which
     * case it must be filled in by yourself.
     *
     * @see get_configuration()
     */
    IModule(const Config& configuration);




    /**
     * @brief Does nothing
     */
    virtual ~IModule() = default;




    /**
     * @brief Method for obtaining module configuration
     * @return `Json::Value&`
     */
    virtual const Config& get_configuration() const noexcept;




    /**
     * @brief Get module data to send to the client
     * @note Use `make_root_node`, `make_container_node` or `make_value_node` to form correct json.
     * See documentation or [this
     * guide](https://github.com/maksimshchavelev/server-monitoring-utility/blob/master/server/for-developers/own_module.md)
     * @see make_root_node
     * @see make_container_node
     * @see make_value_node
     * @return `std::vector<uint8_t>` with bytes in MDTP protocol
     */
    virtual std::optional<std::vector<uint8_t>> get_data() = 0;




    /**
     * @brief Enable module
     */
    virtual void enable();




    /**
     * @brief Disable module
     */
    virtual void disable();




    /**
     * @brief Is module enabled
     * @return `true` if module is enabled, otherwise `false`
     */
    virtual bool is_enabled() const;




    /**
     * @brief Sets poll ratio
     *
     * For example, a value of 5 means that the module will be polled by the server core every fifth
     * polling cycle. Thus, if the server settings specify a polling interval of 1 second, the
     * module will be polled at intervals of 5 seconds.
     */
    virtual void set_poll_ratio(uint32_t poll_ratio);




    /**
     * @brief Get poll ratio
     * @return Poll ratio
     * @see set_poll_ratio
     */
    virtual uint32_t get_poll_ratio() const;




    /**
     * @brief Get module name
     * @note You do not need to implement this method because the `REGISTER_MODULE` macro implements
     * it
     * @return `const std::string_view` with module name
     */
    virtual constexpr std::string_view module_name() const noexcept = 0;




    /**
     * @brief Get module description
     * @note You do not need to implement this method because the `REGISTER_MODULE` macro implements
     * it
     * @return `const std::string_view` with module description
     */
    virtual constexpr std::string_view module_description() const noexcept = 0;



  protected:
    Config m_configuration;  ///< Configuration of module
    bool   m_enabled{false}; ///< Status of module

    uint32_t m_poll_ratio{1};
    ///< Affects the module polling frequency. For example, a value of `5` means that the module
    ///< will be polled by the server core every fifth polling cycle. Thus, if the server settings
    ///< specify a polling interval of `1` second, the module will be polled at intervals of `5`
    ///< seconds.
    ///<
    ///< If the value is `0`, the server core will cache the result of the first query (data
    ///< obtained via `get_data`) and the module will no longer be queried.


    // =============================== LOGGER ===============================

    /**
     * @brief Describes log type. Affects the color of messages
     */
    enum class LogType {
        INFO,    ///< **White** color of logs
        WARNING, ///< **Yellow** color of logs
        ERROR    ///< **Red** color of logs
    };




    /**
     * @brief Outputs the log
     * @param log_type Log type. Takes the following values:
     * LogType::INFO - white log
     * LogType::INFO - yellow log
     * LogType::ERROR - red log
     * @param message Message to log
     * @note Prints white message if `log_type` is incorrect
     *
     * @section example_usage Example usage
     *
     * This code is for a module named "RAM" runned at **04.07.2025 18:28:00**:
     * @code{.cpp}
     * log(LogType::Warning, "Warning log message");
     * @endcode
     *
     * Produces the following output:
     * <div style="background-color:#282c34; color:#ffffff; padding:6px 10px; border:1px solid #444;
     * border-radius:4px; font-family:monospace; font-size:smaller; font-weight:normal;">
     * [<span style="color:#00e5ff;">04.07.25 18:28:00</span>] [MODULE <span class="no-link"
     * style="color:#00e5ff;">RAM</span>] <span style="color:#ffd700;">Warning log message</span>
     * </div>
     */
    void log(LogType log_type, const std::string_view message) const;


  private:
    // Counter of completed poll cycles. When it equals `m_poll_ratio - 1`, a poll occurs.
    // The server core changes it itself, thanks to the mechanism of friendly functions. The field
    // is located in the private section so that the inheriting class (implementation of a specific
    // module) cannot see it.
    uint32_t m_poll_counter{0};

    friend class Application;
};

} // end of namespace smu_server
