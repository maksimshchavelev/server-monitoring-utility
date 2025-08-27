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
 * @brief Struct to storing MDTP data. See documentation for MDTP protocol.
 * @note This is a **packaged** structure.
 */
typedef struct ABI_MODULE_MDTP_DATA {
    const void* data; ///< Pointer to MDTP bytes
    uint32_t    size; ///< Count of MDTP bytes (size of data)
} ABI_MODULE_MDTP_DATA;


/**
 * @brief Context of module
 *
 * The context is needed so that the server core knows what module is accessing it (when calling
 * core functions, for example, logging)
 *
 * @note This is a **packaged** structure
 */
typedef struct ABI_MODULE_CONTEXT {
    const char* module_name;        ///< Name of module
    const char* module_description; ///< Description of module
} ABI_MODULE_CONTEXT;


/**
 * @brief Functions provided by the server for the module
 *
 * This structure will be passed to the module initialization function.
 *
 * @note This is a packaged structure.
 */
typedef struct ABI_SERVER_CORE_FUNCTIONS {
    /**
     * @brief Get ABI version
     * @param context Module context. See `ABI_MODULE_CONTEXT`
     * @return ABI version
     */
    uint32_t (*abi_get_abi_version)(const ABI_MODULE_CONTEXT* context);

    /**
     * @brief Logging function
     *
     * @param context Module context. See `ABI_MODULE_CONTEXT`
     * @param log_type Type of log. Available log types:
     *          - `0`: White (info) log message
     *          - `1`: Yellow (warning) log message
     *          - `2`: Red (error) log message
     * @param message Message to log
     */
    void (*abi_log)(const ABI_MODULE_CONTEXT* context, int log_type, const char* message);
} ABI_SERVER_CORE_FUNCTIONS;


typedef struct ABI_MODULE_FUNCTIONS ABI_MODULE_FUNCTIONS; ///< Forward declaration

/**
 * @brief Functions of the module that calls the server core
 *
 * @warning If functions that return a pointer return `NULL`, it means that an error has occurred.
 *
 * @note This is a packaged structure.
 */
typedef struct ABI_MODULE_FUNCTIONS {
    ABI_MODULE_FUNCTIONS(*module_init)
    (ABI_SERVER_CORE_FUNCTIONS server_functions,
     const char*               json_configuration); ///< Initializes module

    void (*module_destroy)(void); ///< Destroys module

    const char* (*module_get_configuration)(void); ///< Get module json configuration

    const ABI_MODULE_MDTP_DATA* (*module_get_data)(void); ///< Get MDTP module data

    void (*module_enable)(void); ///< Enables a module

    void (*module_disable)(void); ///< Disables a module

    uint8_t (*module_is_enabled)(void); ///< Is module enabled (true/false)

    const char* (*module_get_module_name)(void); ///< Get module name

    const char* (*module_get_module_description)(void); ///< Get module description

    void (*module_set_poll_ratio)(uint32_t poll_ratio); ///< Set the poll ratio of module

    uint32_t (*module_get_poll_ratio)(void); ///< Get poll ratio of module
} ABI_MODULE_FUNCTIONS;



// ====================================== ABI FUNCTIONS ======================================


/**
 * @brief C-callabe function for obtaining the ABI version
 */
extern "C" uint32_t abi_get_abi_version(const ABI_MODULE_CONTEXT*);


/**
 * @brief C-callabe function for logging
 */
extern "C" void abi_log(const ABI_MODULE_CONTEXT* context, int log_type, const char* message);

} // namespace smu_server
