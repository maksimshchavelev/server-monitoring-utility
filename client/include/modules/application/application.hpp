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
 * @brief The Application class to manage other modules
 */
class Application {
  public:
    /**
     * @brief Application constructor
     * @param settings Reference to objcet of `smu::Settings` class
     */
    Application(Settings& settings);




    /**
     * @brief Runs the application and **blocks the main thread**
     * @note Blocks thread
     * @return `0` if success exiting, other error code if error
     *
     * @section example_usage Example usage
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
     * @brief **Force** exit application
     * @param error Error message that will be displayed if an error occurs
     * @note Exits with code `1` if an error occurred and code `0` if no error occurred.
     *
     * @section example_usage Example usage
     * @code{.cpp}
     * app.exit(std::nullopt); // No error. Exiting with status code 0
     * app.exit("Fatal error"); // Display "Fatal error" and exit with status code 1
     * @endcode
     */
    void exit(std::optional<std::string> error);



  private:
    Settings& m_settings; ///< Reference to settings
    Network   m_network;  ///< Network object
    UI        m_ui{};     ///< UI object

    std::atomic_int  m_return_value{0}; ///< Return value. Use in conjunction with m_exit_request
    std::atomic_bool m_exit_request{false}; ///< When it becomes true, the application exits.
    std::condition_variable cw;             ///< for m_exit_request
};

} // namespace smu
