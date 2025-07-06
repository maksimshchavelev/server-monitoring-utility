/// GPLv3 LICENSE, Copyright (©) 2025, Maksim Shchavelev <maksimshchavelev@gmail.com>
/// See LICENSE for details

/**
 * @file core/internals/module.cpp
 * @brief Implementation of Interface Module class
 */

#include "core/internals/module.hpp"




// Public constructor
smu_server::Module::Module(const Json::Value& configuration) : m_configuration(configuration) {}




// Public method
void smu_server::Module::enable() {
    m_running = true;
}




// Public method
void smu_server::Module::disable() {
    m_running = false;
}
