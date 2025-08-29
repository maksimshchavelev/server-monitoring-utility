/// GPLv3 LICENSE, Copyright (©) 2025, Maksim Shchavelev <maksimshchavelev@gmail.com>
/// See LICENSE for details

/**
 * @file external_module_loader/external_module_loader.cpp
 * @brief File with class for load external .so modules
 */

#include "external_module_loader/external_module_loader.hpp"
#include "external_module_loader/internals/proxy_module.hpp"
#include <dlfcn.h>

namespace smu_server {

// Static public method
std::expected<std::unique_ptr<IModule>, std::string> ExternalModuleLoader::load(
    const std::string_view path, const Config& config) {

    // Open shared object
    void* dl_descriptor = dlopen(path.data(), RTLD_NOW);

    // Opening error
    if (dl_descriptor == nullptr) {
        return std::unexpected(dlerror());
    }

    auto module_init_fn =
        reinterpret_cast<ABI_MODULE_FUNCTIONS* (*)(ABI_SERVER_CORE_FUNCTIONS,
                                                   const char* /* configuration */)>(
            dlsym(dl_descriptor, "module_init"));

    // Finding init function error
    if (module_init_fn == nullptr) {
        dlclose(dl_descriptor);
        return std::unexpected(std::format("can't find 'module_init' symbol in {}", path));
    }

    // Init server core functions
    ABI_SERVER_CORE_FUNCTIONS server_core_functions{.abi_get_abi_version = abi_get_abi_version,
                                                    .abi_log = abi_log};

    // Init module
    ABI_MODULE_FUNCTIONS* module_functions =
        module_init_fn(server_core_functions, config.get_json().toStyledString().data());

    // If error occured
    if (module_functions == nullptr) {
        return std::unexpected("'module_init' returned NULL");
    }

    return std::make_unique<internals::ProxyModule>(dl_descriptor, *module_functions, config);
}

} // namespace smu_server
