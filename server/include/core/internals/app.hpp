/// GPLv3 LICENSE, Copyright (©) 2025, Maksim Shchavelev <maksimshchavelev@gmail.com>
/// See LICENSE for details

/**
 * @file core/internals/app.hpp
 * @brief File Application class
 */

#pragma once

#include "module.hpp"
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
        requires std::is_base_of_v<IModule, ModuleType>;


  private:
    Application() = default;

    std::mutex                            m_modules_mutex;
    std::vector<std::unique_ptr<IModule>> m_modules;
};

} // end of namespace smu_server
