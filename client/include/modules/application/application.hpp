/// GPLv3 LICENSE, Copyright (©) 2025, Maksim Shchavelev <maksimshchavelev@gmail.com>
/// See LICENSE for details

/**
 * @file modules/application/application.hpp
 * @brief File with application class to manage other modules
 */

#pragma once

#include "modules/network/network.hpp"
#include "modules/settings/settings.hpp"
#include "modules/ui/ui.hpp"
#include <atomic>
#include <json/json.h>

namespace smu {

/**
 * @brief The Application to manage other modules
 */
class Application {
  public:
    /**
     * @brief Application constructor
     * @param settings Reference to objcet of `smu::Settings` class
     */
    Application(Settings& settings);




    /**
     * @brief Runs the application and block the main thread
     * @return `0` if success exiting, other error code if error
     * @example
     *
     * @code{.cpp}
     *
     * int main() {
     *  // Creating smu::Settings here
     *
     *  smu::Application app(settings);
     *  return app.run();
     * }
     *
     * @endcode
     */
    int run();




    /**
     * @brief Exit application
     * @param error Error message that will be displayed if an error occurs
     * @note Returns code `1` if an error occurred and code `0` if no error occurred.
     */
    void exit(std::optional<std::string> error);



  private:
    Settings& m_settings;
    Network   m_network;
    UI        m_ui{};

    std::atomic_int         m_return_value{0};
    std::atomic_bool        m_exit_request{false};
    std::condition_variable cw; // for m_exit_request




    /**
     * @brief Converts string representation of json to `Json::Value`
     * @param str String representation of json
     * @return `std::expected` with `Json::Value` if success and `std::string` if error
     * @private
     */
    std::expected<Json::Value, std::string> json_from_string(const std::string& str) const noexcept;
};

} // namespace smu
