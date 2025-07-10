/// GPLv3 LICENSE, Copyright (©) 2025, Maksim Shchavelev <maksimshchavelev@gmail.com>
/// See LICENSE for details

/**
 * @file core/internals/app.hpp
 * @brief File Application class
 */

#pragma once

#include "config_manager/config_manager.hpp"
#include "core/controllers/websocket_main_controller.hpp"
#include "module.hpp"
#include <memory>
#include <mutex>
#include <vector>

// We need this to remove compilation warnings inside Drogon
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wuseless-cast"
#include <drogon/drogon.h>
#pragma GCC diagnostic pop

namespace smu_server {

/**
 * @brief The Singleton Application class
 * @details Manages the entire application
 */
class Application {
  public:
    /**
     * @brief Get `Application` instance
     * @return `Application&`
     */
    static Application& instance();




    /**
     * @brief Register module with type `ModuleType`
     */
    template <typename ModuleType>
    void register_module()
        requires std::is_base_of_v<IModule, ModuleType>
    {
        static_assert(
            requires { ModuleType(Json::Value()); },
            "Module must be constructible from const Json::Value&");


        std::cout << std::format("[INFO] Registering module with name \"{}\" (description: \"{}\")",
                                 ModuleType::module_name_static(),
                                 ModuleType::module_description_static())
                  << std::endl;

        auto config = ConfigManager::instance().get_module_config(ModuleType::module_name_static());

        if (config.empty()) {
            // Print yellow warning
            std::cout << std::format(
                             "\033[33m[WARNING] Config of module with name \"{}\" is empty!\033[0m",
                             ModuleType::module_name_static())
                      << std::endl;
        }

        auto                        module = std::make_unique<ModuleType>(config);
        std::lock_guard<std::mutex> lock(m_modules_mutex);
        m_modules.push_back(std::move(module));

        // Print green success
        std::cout << std::format(
                         "\033[32m[INFO] Successfully registered a module with name \"{}\"\033[0m",
                         ModuleType::module_name_static())
                  << std::endl;
    }




    /**
     * @brief Run the application.
     * @note Blocks main thread
     */
    void run();




    /**
     * @brief Collects all metrics from all modules
     * @return `Json::Value` with collected metrics
     */
    Json::Value collect_metrics();




    /**
     * @brief Runs asynchronous collection and sending of metrics to all connected users
     */
    void run_sending_metrics_async();




    /**
     * @brief Saves all configs
     */
    void save_configs() const noexcept;


  private:
    Application();
    ~Application();

    std::mutex                               m_modules_mutex;
    std::vector<std::unique_ptr<IModule>>    m_modules;
    std::shared_ptr<MainWebsocketController> m_main_ws_controller_ptr;
    Json::Value&                             m_server_config;
};

} // end of namespace smu_server
