/// GPLv3 LICENSE, Copyright (©) 2025, Maksim Shchavelev <maksimshchavelev@gmail.com>
/// See LICENSE for details

/**
 * @file logger/logger.hpp
 * @brief Logger class for logging
 */

#pragma once

#include <string_view>

namespace smu_server {

/**
 * @brief Singleton class for logging
 */
class Logger {
  public:
    /**
     * @brief Log type
     */
    enum class LogType {
        INFO,    ///< **White** color
        WARNING, ///< **Yellow** color
        ERROR,   ///< **Red** color
        SUCCESS, ///< **Green** color
        NONE     ///< **White** color
    };


    /**
     * @brief Logs message colorful depending on `log_type`
     * @param log_type Log Type
     * @param message Message
     * @return `Logger&`
     * @see LogType
     * @see logger
     *
     * @section example_usage Example usage
     * This code runned at 04.07.25 18:28:00:
     *
     * @code{.cpp}
     * logger()
     *      .log(LogType::INFO, "Info")
     *      .log(LogType::WARNING, "Warning")
     *      .log(LogType::ERROR, "Error")
     *      .log(LogType::SUCCESS, "Success");
     * @endcode
     *
     * Will output:
     *
     * <div style="background:#282c34; padding:6px 10px; border:1px solid #444; border-radius:4px;
     * font-family:monospace; font-size:smaller; white-space:pre; line-height:1.2;">
     * [<span style="color:#00e5ff;">04.07.25 18:28:00</span>] Info<br>
     * [<span style="color:#00e5ff;">04.07.25 18:28:00</span>] <span
     * style="color:#ffd700;">Warning</span><br>
     * [<span style="color:#00e5ff;">04.07.25 18:28:00</span>] <span
     * style="color:#ff0000;">Error</span><br>
     * [<span style="color:#00e5ff;">04.07.25 18:28:00</span>] <span
     * style="color:#00ff00;">Success</span>
     * </div>
     */
    Logger& log(LogType log_type, const std::string_view message);




    /**
     * @brief Logs message colorless
     * @brief Equivalent to calling `log(LogType::NONE, message)`
     * @param message Message
     * @return `Logger&`
     * @see logger
     */
    Logger& log_colorless(const std::string_view message);




    /**
     * @brief Logs info (white color)
     * @brief Equivalent to calling `log(LogType::INFO, message)`
     * @param message Message
     * @return `Logger&`
     * @see logger
     */
    Logger& log_info(const std::string_view message);




    /**
     * @brief Logs warning (yellow color)
     * @brief Equivalent to calling `log(LogType::WARNING, message)`
     * @param message Message
     * @return `Logger&`
     * @see logger
     */
    Logger& log_warning(const std::string_view message);




    /**
     * @brief Logs error (red color)
     * @brief Equivalent to calling `log(LogType::ERROR, message)`
     * @param message Message
     * @return `Logger&`
     * @see logger
     */
    Logger& log_error(const std::string_view message);




    /**
     * @brief Logs success (green color)
     * @brief Equivalent to calling `log(LogType::SUCCESS, message)`
     * @param message Message
     * @return `Logger&`
     * @see logger
     */
    Logger& log_success(const std::string_view message);


  private:
    /**
     * @brief Logger private constructor
     */
    Logger() = default;


    /**
     * @brief Get `Logger` instance
     * @return `Logger` instance
     *
     * @section example_usage Example usage
     *
     * @code{.cpp}
     *
     * logger().log_info("Info").log_warning("Warning");
     *
     * @endcode
     */
    friend Logger& logger();
};



/**
 * @brief Get `Logger` instance
 * @return `Logger` instance
 *
 * @section example_usage Example usage
 *
 * @code{.cpp}
 *
 * logger().log_info("Info").log_warning("Warning");
 *
 * @endcode
 */
Logger& logger();


} // namespace smu_server
