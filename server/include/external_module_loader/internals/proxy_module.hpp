/// GPLv3 LICENSE, Copyright (©) 2025, Maksim Shchavelev <maksimshchavelev@gmail.com>
/// See LICENSE for details

/**
 * @file external_module_loader/internals/proxy_module.hpp
 * @brief File with class for load external .so modules
 */

#pragma once

#include "abi/abi.hpp"
#include "core/core.hpp"

namespace smu_server::internals {

/**
 * @brief A proxy module that interacts specifically with ABI, but does not collect any data itself.
 *
 * This is necessary so that the server core does not care what module is in front of it. It only
 * works with the module's API.
 */
class ProxyModule : public IModule {
  public:
    // Do not put REGISTER_MODULE macro because we will configure everything manually

    /**
     * @brief Constructs a proxy module for an already loaded dynamic module object with module_init
     * executed.
     * @param dl_handle Dynamic object descriptor obtained via dlopen
     * @param module_functions Structure with pointers to module functions
     * @param context Module context (name and description)
     * @param cfg Configuration of module
     */
    ProxyModule(void*                dl_handle,
                ABI_MODULE_FUNCTIONS module_functions,
                ABI_CONTEXT*         context,
                const Config&        cfg);

    /**
     * @brief Destroys the module and closes the dynamic object
     */
    ~ProxyModule() override;


    /**
     * @brief Calls `module_get_data` and obtaining data
     * @return `std::optional<Json::Value>`
     */
    std::optional<Json::Value> get_data() override;


    /**
     * @brief Enables module
     */
    void enable() override;


    /**
     * @brief Disables module
     */
    void disable() override;


    /**
     * @brief Sets poll ratio
     * @param poll_ratio Poll ratio
     */
    void set_poll_ratio(uint32_t poll_ratio) override;


    /**
     * @brief Get poll ratio
     * @return Poll ratio
     */
    uint32_t get_poll_ratio() const override;


    /**
     * @brief Get module name
     * @return Module name
     */
    constexpr std::string_view module_name() const noexcept override;


    /**
     * @brief Get module description
     * @return Module description
     */
    constexpr std::string_view module_description() const noexcept override;

  private:
    void* m_dl_handle; ///< Dynamic object descriptor obtained via dlopen

    ABI_MODULE_FUNCTIONS m_module_functions; ///< Structure with pointers to module functions
    ABI_CONTEXT*         m_module_context;   ///< Module context (name and description)
};

} // namespace smu_server::internals
