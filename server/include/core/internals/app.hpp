/// GPLv3 LICENSE, Copyright (©) 2025, Maksim Shchavelev <maksimshchavelev@gmail.com>
/// See LICENSE for details

/**
 * @file core/internals/app.hpp
 * @brief File Application class
 */

#pragma once

#include "config_manager/config_manager.hpp"
#include "core/controllers/websocket_main_controller.hpp"
#include "ipc/ipc.hpp"
#include "module.hpp"
#include <functional>
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
     * @note Used only in `ModuleRegistrar`. Do not use directly. Instead, use `add_module_to_queue`
     * for lazy initialization
     * @see `add_module_to_queue`
     */
    template <typename ModuleType>
    void register_module()
        requires std::is_base_of_v<IModule, ModuleType>
    {
        static_assert(
            requires { ModuleType(Json::Value()); },
            "Module must be constructible from const Json::Value&");


        // Print log
        std::cout << std::format("Registering a module with name "
                                 "\"\033[36m{}\033[0m\" and description \"\033[36m{}\033[0m\"... ",
                                 ModuleType::module_name_static(),
                                 ModuleType::module_description_static());
        std::cout.flush(); // For force printing


        // Getting config
        auto config = ConfigManager::instance().get_module_config(ModuleType::module_name_static());


        if (config.empty()) {
            std::cout << "\033[33mGot empty config. Continuing with default values. \033[0m";
            std::cout.flush(); // For force printing
        }


        // Creating module
        std::unique_ptr<ModuleType> module;
        bool                        creation_failed{false};

        try {
            module = std::make_unique<ModuleType>(config);
        } catch (const std::exception& e) {
            creation_failed = true;
            std::cout << std::format("\033[31mFailed! Cause: {}\033[0m", e.what()) << std::endl;
        }

        if (!creation_failed) {
            // Get colorful status (RUNNING/STOPPED)
            std::string module_status =
                module->is_enabled() ? "\033[32mRUNNING\033[0m" : "\033[31mSTOPPED\033[0m";

            // Print colorful log
            std::cout << std::format("\033[32mRegistered\033[0m ({})", module_status) << std::endl;

            std::lock_guard<std::mutex> lock(m_modules_mutex);
            m_modules.push_back(std::move(module));
        }
    }




    /**
     * @brief Adds module to queue. For lazy module initialization. See details
     * @details Adds `register_function` to the internal vector. When it is time
     * to register a module, each function in the vector that registers the module
     * is called. Thus, `register_function` must call `Application::register_module`
     * *by accepted reference*.
     * @param register_function Registration callback
     */
    void add_module_to_queue(std::function<void(Application&)> register_function);




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

    std::mutex                                     m_modules_mutex;
    std::vector<std::unique_ptr<IModule>>          m_modules;
    std::vector<std::function<void(Application&)>> m_modules_queue; // for lazy init
    std::shared_ptr<MainWebsocketController>       m_main_ws_controller_ptr;
    Json::Value&                                   m_server_config;

    IPC m_ipc; // For interprocess communication with CLI


    // ================================ FOR CLI COMMANDS ================================

    /**
     * @brief Get modules name, status and description
     * @return `std::string`
     */
    std::string list_modules() const;
};

} // end of namespace smu_server
