/**
 * @file main.c
 * @brief UPTIME module
 * @license GPLv3, see LICENSE for details
 * @copyright Copyright (©) 2025, Maksim Shchavelev <maksimshchavelev@gmail.com>
 */

#include "utils.h"                                  // For utils
#include <smu-server/sdk/c/helpers/parson/parson.h> // For Json
#include <smu-server/sdk/c/modules/sdk.h>           // For C SDK
#include <stddef.h>                                 // For NULL
#include <string.h>                                 // For memset

// =====================================================================
// Forward declaration of functions that must be implemented in a module


/**
 * @brief The function destroys the module. At the end it must destroy `module.imodule`
 */
void destroy(void);

/**
 * @brief Here you need to correctly respond to the request to enable the module and enable it
 */
void enable(void);

/**
 * @brief Here you need to correctly respond to the request to turn off the module and turn it off
 */
void disable(void);

/**
 * @brief Here we need to return information about the module state
 * @return `1` if module is enabled, otherwise `0`
 */
uint8_t is_enabled(void);

/**
 * @brief Here we need to return the current module configuration
 * @return NULL-terminated string with Json configuration
 */
const char *get_configuration(void);

/**
 * @brief Here we have to generate data using MDTP protocol and functions like `sdk_mdtp_*` and
 * return it
 * @return `ABI_MODULE_MDTP_DATA *`
 */
const ABI_MODULE_MDTP_DATA *get_data(void);

/**
 * @brief Here we need to return the module name
 * @return NULL-terminated string with module name
 */
const char *get_name(void);

/**
 * @brief Here we need to return the module description
 * @return NULL-terminated string with module description
 */
const char *get_description(void);

/**
 * @brief Here you need to return the poll ratio for the module
 * @return Poll ratio
 */
uint32_t get_poll_ratio(void);

/**
 * @brief Here you need to set the poll ratio
 * @param poll_ratio Required poll ratio
 */
void set_poll_ratio(uint32_t poll_ratio);



// End of forward declaration
// =====================================================================


/**
 * @brief Structure for storing module data like json configuration
 *
 * IModule is a structure that stores the state and parameters of our module. We cannot interact
 * with its fields directly, but we can do this through the `sdk_imodule_*` functions.
 *
 * JSON_Value is the root of the json configuration. The Parson library is used
 */
typedef struct Module {
    IModule     *imodule;       ///< Pointer to IModule
    JSON_Value  *cfg_json_root; ///< Root of json configuration
    JSON_Object *cfg_json_obj;  ///< Json object to interact with values
    char *cfg_json_string; ///< Buffer to store json as a string. It is used to temporarily store a
                           ///< pointer to the string that `json_serialize_to_string_pretty`
                           ///< allocates. We will free this memory in `destroy` or before forming a
                           ///< new string in `get_configuration`
} Module;

static Module module;

/**
 * @brief This is the entry point to the module.
 *
 * Here we initialize the module and return a filled structure with pointers to the module functions
 * so that the server can interact with our module.
 *
 * @param server_functions A structure with pointers to server functions so we can access them via
 * `sdk_utils_*` wrappers
 * @param json_configuration Json module configuration. We can use Parson to work with Json by
 * simply including
 * @return `ABI_MODULE_FUNCTIONS *` if success or `NULL` if error
 */
const ABI_MODULE_FUNCTIONS *module_init(ABI_SERVER_CORE_FUNCTIONS server_functions,
                                        const char               *json_configuration) {
    memset(&module, 0x0, sizeof(Module));

    // We allocate memory for IModule and initialize it
    //
    // Note that the module name must match the name of the target you are creating. Case matters!
    // Also note that to free memory at the end of the module (in the destroy function) you need to
    // call `sdk_imodule_destroy`. This will also free the internal resources of the module. You
    // cannot simply call `free((void *)module.imodule)`
    module.imodule =
        sdk_imodule_create("UPTIME",
                           "Module measuring the continuous operating time of the machine",
                           server_functions,
                           1,
                           1);

    // Return NULL if sdk_imodule_create failed
    if (!module.imodule) {
        return NULL;
    }

    // Parse json next
    module.cfg_json_root = json_parse_string(json_configuration);

    // Parsing error
    if (!module.cfg_json_root) {
        sdk_utils_log(module.imodule, LOG_ERROR, "Error parsing json. Abort");
        return NULL;
    }

    // Read parameters
    module.cfg_json_obj = json_value_get_object(module.cfg_json_root);

    // Error
    if (module.cfg_json_obj == NULL) {
        sdk_utils_log(
            module.imodule, LOG_ERROR, "Failed to get json object via 'json_value_get_object'");
        json_value_free(module.cfg_json_root); // Free memory to avoid leaks
        return NULL;
    }

    // Try to parse state
    if (json_object_has_value_of_type(module.cfg_json_obj, "enabled", JSONBoolean)) {
        uint8_t enabled = (uint8_t)json_object_get_boolean(module.cfg_json_obj, "enabled");
        if (enabled) {
            sdk_imodule_enable(module.imodule);
        } else {
            sdk_imodule_disable(module.imodule);
        }
    } else {
        // Print warning
        sdk_utils_log(
            module.imodule,
            LOG_WARNING,
            "Field 'enabled' not found in json configuration. Module will be enabled by default.");
        sdk_imodule_enable(module.imodule);

        // Also set enabled in configuration
        json_object_set_boolean(module.cfg_json_obj, "enabled", 1);
    }

    // Try to parse poll ratio
    if (json_object_has_value_of_type(module.cfg_json_obj, "poll_ratio", JSONNumber)) {
        uint32_t poll_ratio = (uint32_t)json_object_get_number(module.cfg_json_obj, "poll_ratio");
        sdk_imodule_set_poll_ratio(module.imodule, poll_ratio);
    } else {
        // Print warning
        sdk_utils_log(module.imodule,
                      LOG_WARNING,
                      "Field 'poll_ratio' not found in json configuration. Set 'poll_ratio' to '1' "
                      "by default.");
        sdk_imodule_set_poll_ratio(module.imodule, 1);

        // Also set poll ratio in configuration
        json_object_set_number(module.cfg_json_obj, "poll_ratio", 1);
    }


    // Here we register handlers which the server calls
    sdk_module_register_destroy(module.imodule, destroy);
    sdk_module_register_enable(module.imodule, enable);
    sdk_module_register_disable(module.imodule, disable);
    sdk_module_register_is_enabled(module.imodule, is_enabled);
    sdk_module_register_get_configuration(module.imodule, get_configuration);
    sdk_module_register_get_data(module.imodule, get_data);
    sdk_module_register_get_module_name(module.imodule, get_name);
    sdk_module_register_get_module_description(module.imodule, get_description);
    sdk_module_register_get_poll_ratio(module.imodule, get_poll_ratio);
    sdk_module_register_set_poll_ratio(module.imodule, set_poll_ratio);

    // Return function table
    return sdk_imodule_get_module_functions(module.imodule);
}


// =============================== IMPLEMENTATION OF OTHER FUNCTIONS ===============================

void destroy(void) {
    // Free json string
    json_free_serialized_string(module.cfg_json_string);

    // Free json
    json_value_free(module.cfg_json_root);

    // Finally, destroy the `module`
    sdk_imodule_destroy(module.imodule);
}


void enable(void) {
    sdk_imodule_enable(module.imodule);

    // Change state in configuration
    json_object_set_boolean(module.cfg_json_obj, "enabled", sdk_imodule_is_enabled(module.imodule));
}


void disable(void) {
    sdk_imodule_disable(module.imodule);

    // Change state in configuration
    json_object_set_boolean(module.cfg_json_obj, "enabled", sdk_imodule_is_enabled(module.imodule));
}


uint8_t is_enabled(void) {
    return sdk_imodule_is_enabled(module.imodule);
}


const char *get_configuration(void) {
    // First, free memory
    json_free_serialized_string(module.cfg_json_string);

    // Then, create new string
    module.cfg_json_string = json_serialize_to_string_pretty(module.cfg_json_root);

    // And return
    return module.cfg_json_string;
}


const ABI_MODULE_MDTP_DATA *get_data(void) {
    double uptime = 0.0;
    double idle = 0.0;

    char uptime_buffer[64];
    char idle_buffer[64];

    // Read uptime
    if (utils_read_proc_uptime(&uptime, &idle) == -1) {
        sdk_utils_log(module.imodule, LOG_ERROR, "'utils_read_proc_uptime' returned NULL");
        return NULL; // error occured
    }

    if (utils_seconds_to_dd_hh_mm_ss((uint64_t)uptime, uptime_buffer, sizeof(uptime_buffer)) ==
        -1) {
        sdk_utils_log(module.imodule, LOG_ERROR, "'utils_seconds_to_dd_hh_mm_ss' (for uptime_buffer) returned NULL");
        return NULL; // error occured
    }

    if (utils_seconds_to_dd_hh_mm_ss((uint64_t)idle, idle_buffer, sizeof(idle_buffer)) == -1) {
        sdk_utils_log(module.imodule, LOG_ERROR, "'utils_seconds_to_dd_hh_mm_ss' (for idle buffer) returned NULL");
        return NULL; // error occured
    }

    // Form MDTP
    return sdk_mdtp_make_root(module.imodule,
                              sdk_mdtp_make_value("UPTIME", uptime_buffer, ""),
                              sdk_mdtp_make_value("IDLE", idle_buffer, ""),
                              NULL);
}


const char *get_name(void) {
    // We get the module context and extract its name from it.
    return sdk_imodule_get_context(module.imodule)->module_name;
}


const char *get_description(void) {
    // We get the module context and extract its description from it.
    return sdk_imodule_get_context(module.imodule)->module_description;
}


uint32_t get_poll_ratio(void) {
    return sdk_imodule_get_poll_ratio(module.imodule);
}


void set_poll_ratio(uint32_t poll_ratio) {
    sdk_imodule_set_poll_ratio(module.imodule, poll_ratio);

    json_object_set_number(
        module.cfg_json_obj, "poll_ratio", sdk_imodule_get_poll_ratio(module.imodule));
}
