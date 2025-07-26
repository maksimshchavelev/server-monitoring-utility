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
        INFO,    /// white color
        WARNING, /// yellow color
        ERROR,   /// red color
        SUCCESS, /// green color
        NONE     /// white color
    };


    /**
     * @brief Logs message colorful depending on `log_type`
     * @param log_type Log Type
     * @param message Message
     * @return `Logger&`
     * @see LogType
     * @see logger
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
     * @brief Friend factory function to get logger
     * @return `Logger`
     */
    friend Logger& logger();
};



/**
 * @brief Get logger instance
 * @return `Logger` instance
 * @example
 *
 * @code{.cpp}
 *
 * logger().log_info("Info").log_warning("Warning");
 *
 * @endcode
 */
Logger& logger();


} // namespace smu_server
