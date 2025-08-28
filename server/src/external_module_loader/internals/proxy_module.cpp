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
                         const Config&        cfg) :
    IModule(cfg), m_dl_handle(dl_handle), m_module_functions(module_functions) {

    // Is enabled?
    if (m_module_functions.module_is_enabled == nullptr) {
        log(LogType::ERROR,
            "ProxyModule(): failed to call 'module_is_enabled' because it points to NULL. "
            "'m_enabled' is set to 'true' by default");
        m_enabled = true;
    } else {
        m_enabled = m_module_functions.module_is_enabled();
    }

    // Poll ratio
    if (m_module_functions.module_get_poll_ratio == nullptr) {
        log(LogType::ERROR,
            "ProxyModule(): failed to call 'module_get_poll_ratio' because it points to NULL. "
            "'m_poll_ratio' is set to '1' by default");
        m_poll_ratio = 1;
    } else {
        m_poll_ratio = m_module_functions.module_get_poll_ratio();
    }
}


// Public destructor
ProxyModule::~ProxyModule() {
    m_module_functions.module_destroy();
    dlclose(m_dl_handle);
}


// Public method
std::optional<std::vector<uint8_t>> ProxyModule::get_data() {
    if (m_module_functions.module_get_data == nullptr) {
        log(LogType::ERROR,
            "get_data(): failed to call 'module_get_data' because it points to NULL.");
        return std::nullopt;
    }

    const ABI_MODULE_MDTP_DATA* data = m_module_functions.module_get_data();

    if (data == nullptr) {
        log(LogType::ERROR,
            "get_data(): call to 'module_get_data' failed because the return value is NULL.");
        return std::nullopt;
    }

    return std::vector<uint8_t>(static_cast<const uint8_t*>(data->data),
                                static_cast<const uint8_t*>(data->data) + data->size);
}


// Public method
const Config& ProxyModule::get_configuration() const noexcept {
    if (m_module_functions.module_get_configuration == nullptr) {
        log(LogType::ERROR,
            "get_configuration(): failed to call 'module_get_configuration' because it points to "
            "NULL");
        return m_configuration;
    }

    const char* configuration = m_module_functions.module_get_configuration();
    if (configuration == nullptr) {
        log(LogType::ERROR,
            "get_configuration(): call to 'module_get_configuration' failed because the return "
            "value is NULL");
        return m_configuration;
    }

    // Converting string to json
    const auto     raw_json_length = static_cast<int>(strlen(configuration));
    JSONCPP_STRING err;
    Json::Value    config;

    Json::CharReaderBuilder                 builder;
    const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
    if (!reader->parse(configuration, configuration + raw_json_length, &config, &err)) {
        log(LogType::ERROR, "get_configuration(): failed to parse json from module, cause: " + err);
        return m_configuration;
    }

    m_configuration = Config(config);
    return m_configuration;
}


// Public method
void ProxyModule::enable() {
    if (m_module_functions.module_enable == nullptr) {
        log(LogType::ERROR, "enable(): failed to call 'module_enable' because it points to NULL.");
        return;
    }

    m_module_functions.module_enable();
}


// Public method
void ProxyModule::disable() {
    if (m_module_functions.module_disable == nullptr) {
        log(LogType::ERROR,
            "disable(): failed to call 'module_disable' because it points to NULL.");
        return;
    }

    m_module_functions.module_disable();
}


// Public method
bool ProxyModule::is_enabled() const {
    if (m_module_functions.module_is_enabled == nullptr) {
        log(LogType::ERROR,
            "is_enabled(): failed to call 'module_is_enabled' because it points to NULL. Returning "
            "'true'");
        return true;
    }

    return static_cast<bool>(m_module_functions.module_is_enabled());
}


// Public method
void ProxyModule::set_poll_ratio(uint32_t poll_ratio) {
    if (m_module_functions.module_set_poll_ratio == nullptr) {
        log(LogType::ERROR,
            "set_poll_ratio(): failed to call 'module_set_poll_ratio' because it points to NULL.");
        return;
    }

    m_module_functions.module_set_poll_ratio(poll_ratio);
}


// Public method
uint32_t ProxyModule::get_poll_ratio() const {
    if (m_module_functions.module_get_poll_ratio == nullptr) {
        log(LogType::ERROR,
            "get_poll_ratio(): failed to call 'module_get_poll_ratio' because it points to NULL. "
            "Returning '1'");
        return 1;
    }

    return m_module_functions.module_get_poll_ratio();
}


// Public method
constexpr std::string_view ProxyModule::module_name() const noexcept {
    if (m_module_functions.module_get_module_name == nullptr) {
        log(LogType::ERROR,
            "module_name(): failed to call 'module_get_module_name' because it points to NULL. "
            "Returning 'UNNAMED'");
        return "UNNAMED";
    }

    if (const char* module_name = m_module_functions.module_get_module_name();
        module_name == nullptr) {
        log(LogType::ERROR,
            "module_name(): call to 'module_get_module_name' failed because the return "
            "value is NULL. Returning 'UNNAMED'");
        return "UNNAMED";
    } else {
        return module_name;
    }
}


// Public method
constexpr std::string_view ProxyModule::module_description() const noexcept {
    if (m_module_functions.module_get_module_description == nullptr) {
        log(LogType::ERROR,
            "module_description(): failed to call 'module_get_module_description' because it "
            "points to NULL. Returning 'NO DESCRIPTION'");
        return "NO DESCRIPTION";
    }

    if (const char* module_desc = m_module_functions.module_get_module_name();
        module_desc == nullptr) {
        log(LogType::ERROR,
            "module_description(): call to 'module_get_module_description' failed because the "
            "return "
            "value is NULL. Returning 'NO DESCRIPTION'");
        return "NO DESCRIPTION";
    } else {
        return module_desc;
    }
}

} // namespace smu_server::internals
