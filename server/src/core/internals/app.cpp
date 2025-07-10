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
    drogon::app().addListener("0.0.0.0", 5050).registerController(m_main_ws_controller_ptr);
    drogon::app().getLoop()->runAfter(0.0, [this](){ run_sending_metrics_async(); });
    drogon::app().run();
}




// Public method
Json::Value smu_server::Application::collect_metrics() {
    Json::Value root;

    std::lock_guard<std::mutex> lock(m_modules_mutex);
    for (const auto& module : m_modules) {
        if (auto module_data = module->get_data(); module_data.has_value()) {
            root[module->module_name().data()] = std::move(module_data.value());
        }
    }

    return root;
}




// Public method
void smu_server::Application::run_sending_metrics_async() {
    static bool running{false};

    if (running) {
        LOG_WARN << "Application::run_sending_metrics_async() is already running. Skipping run "
                    "again request";
        return;
    }

    running = true;
    std::thread runner([this]() {
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(1)); // sleep for 1 second

            if (m_main_ws_controller_ptr->get_connections_count() > 0) {
                auto metrics = collect_metrics();
                m_main_ws_controller_ptr->send_everyone(metrics);
            }
        }
    });

    runner.detach();
}




// Private constructor
smu_server::Application::Application() :
    m_main_ws_controller_ptr(std::make_shared<MainWebsocketController>()) {}
