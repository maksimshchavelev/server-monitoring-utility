/// GPLv3 LICENSE, Copyright (©) 2025, Maksim Shchavelev <maksimshchavelev@gmail.com>
/// See LICENSE for details

/**
 * @file core/internals/module.cpp
 * @brief Implementation of Interface Module class
 */

#include "core/internals/module.hpp"
#include "logger/logger.hpp"



// Public constructor
smu_server::IModule::IModule(const smu_server::Config& configuration) :
    m_configuration(configuration) {}




// Public method
const smu_server::Config& smu_server::IModule::get_configuration() const noexcept {
    return m_configuration;
}




// Public method
void smu_server::IModule::enable() {
    m_enabled = true;
}




// Public method
void smu_server::IModule::disable() {
    m_enabled = false;
}




// Public method
bool smu_server::IModule::is_enabled() const noexcept {
    return m_enabled;
}




// Protected method
void smu_server::IModule::log(LogType log_type, std::string_view message) const {
    const char* color = nullptr;

    switch (log_type) {
    case LogType::INFO:
        color = "\033[37m";
        break;
    case LogType::WARNING:
        color = "\033[33m";
        break;
    case LogType::ERROR:
        color = "\033[31m";
        break;
    default:
        color = "\033[0m";
        break;
    }

    // For example: [MODULE RAM] Initialization error!
    logger().log_colorless(
        std::format("[MODULE \033[36m{}\033[0m] {}{}\033[0m", module_name(), color, message));
}
