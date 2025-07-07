/// GPLv3 LICENSE, Copyright (©) 2025, Maksim Shchavelev <maksimshchavelev@gmail.com>
/// See LICENSE for details

/**
 * @file core/internals/app.cpp
 * @brief Implementation of the Application class
 */


#include "core/internals/app.hpp"


// Public constructor
smu_server::Application& smu_server::Application::instance() {
    static Application app;
    return app;
}




// Public method
template <typename ModuleType>
void smu_server::Application::register_module()
    requires std::is_base_of_v<IModule, ModuleType>
{
    std::lock_guard<std::mutex> lock(m_modules_mutex);

    ModuleType* module = new ModuleType(Json::Value());
    LOG_INFO << std::format("Registering module with name {} (description: {})", module->module_name(),
                            module->module_description());

    m_modules.emplace_back(module);
}



// Public method
void smu_server::Application::run() {
    drogon::app().addListener("0.0.0.0", 5050).run();
}
