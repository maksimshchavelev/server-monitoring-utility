/// GPLv3 LICENSE, Copyright (©) 2025, Maksim Shchavelev <maksimshchavelev@gmail.com>
/// See LICENSE for details

/**
 * @file abi/abi.hpp
 * @brief File with ABI For interaction between modules in different languages and the server
 */

#pragma once

#include <cstdint>

namespace smu_server {

constexpr int ABI_VERSION = 1; ///< Version of ABI

/**
 * @brief The context in which the module calls server core functions
 *
 * For example, for the logging function, you need to pass the context with the name of the module
 * that will be displayed in the logs.
 *
 * @note This is a packaged structure.
 */
struct __attribute__((packed)) ABI_CONTEXT {
    const char* module_name;
    const char* module_description;
};


/**
 * @brief Functions provided by the server for the module
 *
 * This structure will be passed to the module initialization function.
 *
 * @note This is a packaged structure.
 */
struct __attribute__((packed)) ABI_SERVER_CORE_FUNCTIONS {
    void (*abi_log)(ABI_CONTEXT* context,
                    int          log_type,
                    const char*  message); ///< Logging function. Log types:
                                          ///< `0`: White (info) log message
                                          ///< `1`: Yellow (warning) log message
                                          ///< '2': Red (error) log message

    int (*abi_get_abi_version)(); ///< Returns ABI version
};


/**
 * @brief Functions of the module that calls the server core
 *
 * @warning If functions that return a pointer return `NULL`, it means that an error has occurred.
 *
 * @note This is a packaged structure.
 */
struct __attribute__((packed)) ABI_MODULE_FUNCTIONS {
    ABI_MODULE_FUNCTIONS(*module_init)
    (ABI_SERVER_CORE_FUNCTIONS server_functions,
     const char*               json_configuration); ///< Initializes module

    void (*module_destroy)(); ///< Destroys module

    const char* (*module_get_configuraion)(); ///< Get module json configuration

    const char* (*module_get_data)(); ///< Get json module data

    void (*module_enable)(); ///< Enables a module

    void (*module_disable)(); ///< Disables a module

    bool (*module_is_enabled)(); ///< Is module enabled (true/false)

    void (*module_set_poll_ratio)(uint32_t poll_ratio); ///< Set poll ratio of module

    uint32_t (*module_get_poll_ratio)(); ///< Get poll ratio of mobule

    const char* (*module_get_module_name)(); ///< Get module name

    const char* (*module_get_module_description)(); ///< Get module description
};




// ====================================== ABI FUNCTIONS ======================================


/**
 * @brief C-callabe function for obtaining the ABI version
 */
extern "C" int abi_get_abi_version();


/**
 * @brief C-callabe function for logging
 */
extern "C" void abi_log(ABI_CONTEXT* context, int log_type, const char* message);

} // namespace smu_server
