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
void smu_server::Application::run() {
    {
        std::lock_guard<std::mutex> lock(m_modules_mutex);

        // Print info about modules
        LOG_INFO << std::format("Found {} modules", m_modules.size());
        for (const auto& module : m_modules) {
            LOG_INFO << std::format("Found module with name {} (description: {})",
                                    module->module_name(),
                                    module->module_description());
        }
    }


    drogon::app().addListener("0.0.0.0", 5050).run();
}
