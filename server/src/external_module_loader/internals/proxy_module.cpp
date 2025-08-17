/// GPLv3 LICENSE, Copyright (©) 2025, Maksim Shchavelev <maksimshchavelev@gmail.com>
/// See LICENSE for details

/**
 * @file external_module_loader/internals/proxy_module.cpp
 * @brief File with class for load external .so modules
 */

#include "external_module_loader/internals/proxy_module.hpp"
#include <dlfcn.h> // for dlclose

namespace smu_server::internals {

// Public constructor
ProxyModule::ProxyModule(void*                dl_handle,
                         ABI_MODULE_FUNCTIONS module_functions,
                         ABI_CONTEXT*         context,
                         const Config&        cfg) :
    IModule(cfg), m_dl_handle(dl_handle), m_module_functions(module_functions),
    m_module_context(context) {

    m_enabled = m_module_functions.module_is_enabled();
    m_poll_ratio = m_module_functions.module_get_poll_ratio();
}


// Public destructor
ProxyModule::~ProxyModule() {
    m_module_functions.module_destroy();
    dlclose(m_dl_handle);
}


// Public method
std::optional<std::vector<uint8_t>> ProxyModule::get_data() {
    ABI_MDTP_DATA data = m_module_functions.module_get_data();

    return std::vector<uint8_t>(data.data, data.data + data.size);
}


// Public method
void ProxyModule::enable() {
    m_module_functions.module_enable();
    m_enabled = true;
}


// Public method
void ProxyModule::disable() {
    m_module_functions.module_disable();
    m_enabled = false;
}


// Public method
void ProxyModule::set_poll_ratio(uint32_t poll_ratio) {
    m_module_functions.module_set_poll_ratio(poll_ratio);
    m_poll_ratio = poll_ratio;
}


// Public method
uint32_t ProxyModule::get_poll_ratio() const {
    return m_poll_ratio;
}


// Public method
constexpr std::string_view ProxyModule::module_name() const noexcept {
    return m_module_context->module_name;
}


// Public method
constexpr std::string_view ProxyModule::module_description() const noexcept {
    return m_module_context->module_description;
}

} // namespace smu_server::internals
