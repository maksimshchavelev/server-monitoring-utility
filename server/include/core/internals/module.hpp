/// GPLv3 LICENSE, Copyright (©) 2025, Maksim Shchavelev <maksimshchavelev@gmail.com>
/// See LICENSE for details

/**
 * @file core/internals/module.hpp
 * @brief File with Module class
 */

#pragma once

#include <json/json.h>
#include <optional>

namespace smu_server {

/**
 * @brief The Interface Module class
 * @details Used to obtain information about a specific hardware part of the device
 */
class Module {
  public:
    /**
     * @brief Module
     *
     * @param configuration Json::Value with module configuration. See details
     *
     * @details The configuration of the module is stored in a common configuration file,
     * when the module is constructed, it is passed a fragment with the configuration.
     * When the configuration is saved (e.g., when the application exits), it is fetched
     * via the `get_configuration()` method
     *
     * @warning When the program is first run, an empty Json::Value is passed, in which
     * case it must be filled in by yourself. Below are the requirements for json:
     * - Must contain a `name` field that clearly reflects the purpose of the module.
     *
     * @see `get_configuration()`
     */
    Module(const Json::Value& configuration);




    /**
     * @brief Does nothing
     */
    virtual ~Module() = default;




    /**
     * @brief Method for obtaining module configuration
     * @return `Json::Value&`
     * @note Implement this method in the inheritor class
     */
    virtual const Json::Value& get_configuration() const = 0;

    /**
     * @brief Get module data to send to the client
     *
     * @details Suppose the module monitors CPU load, then it should render json like this:
     *
     * ```
     * {
     *  "core1": {
     *      "type": "value"
     *      "displayed_name": "Core 1",
     *      "value": 50,
     *      "unit": "percents"
     *  },
     *  "core2": {
     *      "type": "value"
     *      "displayed_name": "Core 2",
     *      "value": 2,
     *      "unit": "percents"
     *  }
     * }
     *
     * ```
     * Keep in mind that json consists of nested objects that are either containers
     * (into other objects) or values (this is done for ease of parsing and building
     * client-side UI). * Either way, they must have fields `displayed_name`
     * (name displayed on the client side) and `type` (type of object - `container`
     * or `value`).
     *
     * @return std::optional<Json::Value> with Json::Value inside *if module is
     * running*
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

  protected:
    Json::Value m_configuration;
    bool        m_running{false};
};

} // end of namespace smu_server
