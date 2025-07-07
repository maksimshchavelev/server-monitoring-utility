/// GPLv3 LICENSE, Copyright (©) 2025, Maksim Shchavelev <maksimshchavelev@gmail.com>
/// See LICENSE for details

/**
 * @file core/internals/module.cpp
 * @brief Implementation of Interface Module class
 */

#include "core/internals/module.hpp"




// Public constructor
smu_server::IModule::IModule(const Json::Value& configuration) : m_configuration(configuration) {}




// Public method
void smu_server::IModule::enable() {
    m_running = true;
}




// Public method
void smu_server::IModule::disable() {
    m_running = false;
}




// Public method
bool smu_server::IModule::is_enabled() const {
    return m_running;
}
