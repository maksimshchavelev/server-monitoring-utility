/// GPLv3 LICENSE, Copyright (©) 2025, Maksim Shchavelev <maksimshchavelev@gmail.com>
/// See LICENSE for details

/**
 * @file core/internals/app.hpp
 * @brief File Application class
 */

#pragma once

#include "module.hpp"
#include <drogon/drogon.h>
#include <memory>
#include <mutex>
#include <vector>

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

        ModuleType* module = new ModuleType(Json::Value());
        std::cout << std::format("Registered module with name \"{}\" (description: \"{}\")",
                                 module->module_name(),
                                 module->module_description())
                  << std::endl;

        std::lock_guard<std::mutex> lock(m_modules_mutex);
        m_modules.emplace_back(module);
    }




    /**
     * @brief Run the application.
     * @note Blocks main thread
     */
    void run();


  private:
    Application() = default;

    std::mutex                            m_modules_mutex;
    std::vector<std::unique_ptr<IModule>> m_modules;
};

} // end of namespace smu_server
