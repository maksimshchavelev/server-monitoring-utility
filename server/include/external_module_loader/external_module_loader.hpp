/// GPLv3 LICENSE, Copyright (©) 2025, Maksim Shchavelev <maksimshchavelev@gmail.com>
/// See LICENSE for details

/**
 * @file external_module_loader/external_module_loader.hpp
 * @brief File with class for load external .so modules
 */

#pragma once

#include "core/core.hpp"

namespace smu_server {

/**
 * @brief Class for loading external .so modules
 */
class ExternalModuleLoader {
  public:
    /**
     * @brief Loads external dynamic module
     * @param path Path to .so (for example, `modules/CPU.so`)
     * @param config The configuration that will be transferred to the module
     * @return `std::expected` with pointer to `IModule` if success, otherwise error description
     */
    static std::expected<std::unique_ptr<IModule>, std::string> load(const std::string_view path,
                                                                     const Config&          config);
};

} // namespace smu_server
