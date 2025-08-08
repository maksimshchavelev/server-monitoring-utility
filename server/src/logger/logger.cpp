/// GPLv3 LICENSE, Copyright (©) 2025, Maksim Shchavelev <maksimshchavelev@gmail.com>
/// See LICENSE for details

/**
 * @file logger/logger.cpp
 * @brief Logger class for logging
 */

#include "logger/logger.hpp"
#include <ctime>
#include <string>
#include <unistd.h>

namespace smu_server {

// Public method
Logger& Logger::log(LogType log_type, const std::string_view message) {
    // Color
    const char* color;

    switch (log_type) {
    case LogType::INFO:
        color = "\033[0m"; // default
        break;

    case LogType::WARNING:
        color = "\033[33m";
        break;

    case LogType::ERROR:
        color = "\033[31m";
        break;

    case LogType::SUCCESS:
        color = "\033[32m";
        break;

    default:
        color = "\033[0m";
        break;
    }


    // Time
    char        timebuf[32];
    std::time_t t = std::time(nullptr);
    std::tm     tm{};
    localtime_r(&t, &tm);
    std::strftime(timebuf, sizeof(timebuf), "[\033[36m%d.%m.%y %H:%M:%S\033[0m] ", &tm);

    // Output string
    std::string out;
    out.reserve(256);
    out.append(timebuf);
    out.append(color);
    out.append(message.data(), message.size());
    out.append("\033[0m");
    out.push_back('\n');

    // Print
    write(STDOUT_FILENO, out.c_str(), out.size());

    return *this;
}




// Public method
Logger& Logger::log_colorless(const std::string_view message) {
    return log(LogType::NONE, message);
}




// Public method
Logger& Logger::log_info(const std::string_view message) {
    return log(LogType::INFO, message);
}




// Public method
Logger& Logger::log_warning(const std::string_view message) {
    return log(LogType::WARNING, message);
}




// Public method
Logger& Logger::log_error(const std::string_view message) {
    return log(LogType::ERROR, message);
}




// Public method
Logger& Logger::log_success(const std::string_view message) {
    return log(LogType::SUCCESS, message);
}




// External factory function
Logger& logger() {
    static Logger logger;
    return logger;
}



} // namespace smu_server
