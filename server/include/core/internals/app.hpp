/// GPLv3 LICENSE, Copyright (©) 2025, Maksim Shchavelev <maksimshchavelev@gmail.com>
/// See LICENSE for details

/**
 * @file core/internals/app.hpp
 * @brief File Application class
 */

#pragma once

#include "cli/cli.hpp"
#include "config_io/config_io.hpp"
#include "logger/logger.hpp"
#include "module.hpp"
#include "network/network.hpp"
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
        logger().log_colorless(
            std::format("Registering a module with name "
                        "\"\033[36m{}\033[0m\" and description \"\033[36m{}\033[0m\"...",
                        ModuleType::module_name_static(),
                        ModuleType::module_description_static()));

        // Getting config
        auto config = ConfigIO::instance().get_module_config(ModuleType::module_name_static());

        // For example, [MODULE RAM]
        const std::string module_log_prefix =
            std::format("[MODULE \033[36m{}\033[0m] ", ModuleType::module_name_static());

        if (config.empty()) {
            logger().log_colorless(
                std::format("{}\033[33mGot empty config. Continuing with default values\033[30m",
                            module_log_prefix));
        }


        // Creating module
        std::unique_ptr<ModuleType> module;
        bool                        creation_failed{false};

        try {
            module = std::make_unique<ModuleType>(std::move(config));
        } catch (const std::exception& e) {
            creation_failed = true;
            logger().log_colorless(std::format(
                "{}\033[31mRegistration failed! Cause: {}\033[30m\n", module_log_prefix, e.what()));
        }

        if (!creation_failed) {
            // Get colorful status (RUNNING/STOPPED)
            std::string module_status =
                module->is_enabled() ? "\033[32mRUNNING\033[0m" : "\033[31mSTOPPED\033[0m";

            // Print colorful log
            logger().log_colorless(std::format(
                "{}\033[32mRegistered\033[0m ({})\n", module_log_prefix, module_status));

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
     * @note Blocks main thread. May exit depending on argv (e.g. `--version` or `--help` key
     * received)
     * @param argc Count of command line arguments
     * @param argv Values of command line arguments
     */
    void run(int argc, char** argv);




  private:
    Application();
    ~Application();

    std::mutex                                     m_modules_mutex;
    std::vector<std::unique_ptr<IModule>>          m_modules;
    std::vector<std::function<void(Application&)>> m_modules_queue; // for lazy init
    Config                                         m_server_config;

    friend class CLI; // CLI has access to all Application fields and methods
    // Heavy objects (and which may throw an exception) should be created in `run`
    CLI m_cli; // For interprocess communication with CLI

    Network m_network; // network

    // The flag is needed so that we don't save the config if we started the server with a key that
    // is not supposed to run (such as version or help output). Without this key, the error of
    // saving the config is output in the destructor (because we run without superuser rights).
    bool m_need_save_config_in_destructor{true};


    /**
     * @brief Parses command line arguments
     * @param argc Count of command line arguments
     * @param argv Values of command line arguments
     * @return `true` if arguments that imply server termination are parsed, such as `--version` or
     * `--help`, otherwise (or when parsing error) false
     * @note Can print text (help, version or error...)
     */
    bool parse_argv(int argc, char** argv) const noexcept;




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
};

} // end of namespace smu_server
