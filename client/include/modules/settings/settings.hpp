/// GPLv3 LICENSE, Copyright (©) 2025, Maksim Shchavelev <maksimshchavelev@gmail.com>
/// See LICENSE for details

/**
 * @file modules/settings/settings.hpp
 * @brief File with settings to setup when startup
 */

#pragma once

#include <cstdint>
#include <expected>
#include <string>

namespace smu {

/**
 * @brief Settings class to setup application on startup
 */
class Settings {
  public:
    /**
     * @brief Constructor from command line arguments
     * @note should_exit returns true if you should exit
     * (e.g., a print version or help request is received, not a startup option)
     * @param argc Count of command-line arguments
     * @param argv Command line arguments
     * @throw Throws `std::runtime_error` with description if parsing error
     */
    Settings(int argc, char** argv);




    /**
     * @brief Should the program be terminated?
     * @details You should terminate the program if a request to print a version or help
     * (anything not related to the program launch) is passed through the command line arguments
     * @return `true` if yes, otherwise `false`
     */
    bool should_exit() const noexcept;




    /**
     * @brief Get connection port
     * @return Port
     */
    uint16_t get_port() const noexcept;




    /**
     * @brief Get connection IP
     * @return `std::string_view` with IP
     */
    const std::string_view get_ip() const noexcept;




  private:
    /**
     * @brief Parses command line arguments and changes fields
     * @param argc Count of command-line arguments
     * @param argv Command line arguments
     * @return `std::expected` with void if success, `std::string` with error description
     * if error
     */
    std::expected<void, std::string> parse(int argc, char** argv) noexcept;




    bool m_should_exit{false};


    // Parameters
    uint16_t    m_port; // connection port
    std::string m_ip;   // connection IP
};

} // namespace smu
