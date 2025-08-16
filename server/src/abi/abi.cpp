/// GPLv3 LICENSE, Copyright (©) 2025, Maksim Shchavelev <maksimshchavelev@gmail.com>
/// See LICENSE for details

/**
 * @file abi/abi.cpp
 * @brief File with ABI For interaction between modules in different languages and the server
 */

#include "abi/abi.hpp"
#include "logger/logger.hpp"
#include <format>

namespace smu_server {

extern "C" int abi_get_abi_version(ABI_CONTEXT*) {
    return ABI_VERSION;
}


void abi_log(ABI_CONTEXT* context, int log_type, const char* message) {
    if (context == nullptr || context->module_name == nullptr || message == nullptr) {
        return;
    }

    const char* color = nullptr;

    switch (log_type) {
    case 0:
        color = "\033[37m";
        break;
    case 1:
        color = "\033[33m";
        break;
    case 2:
        color = "\033[31m";
        break;
    default:
        color = "\033[0m";
        break;
    }

    // For example: [MODULE RAM] Initialization error!
    logger().log_colorless(std::format(
        "[MODULE \033[36m{}\033[0m] {}{}\033[0m", context->module_name, color, message));
}

} // namespace smu_server
