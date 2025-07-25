/// GPLv3 LICENSE, Copyright (©) 2025, Maksim Shchavelev <maksimshchavelev@gmail.com>
/// See LICENSE for details

/**
 * @file cli/cli.cpp
 * @brief File with smu-cli command receiver
 */

#include "cli/cli.hpp"
#include "compile-time_config.hpp"
#include "core/internals/app.hpp"

namespace smu_server {

// Public constructor
CLI::CLI(Application& app) : m_app(app) {}




// Public method
void CLI::run() {
    m_ipc.emplace(IPC(ABSTRACT_SOCKET_NAME));

    // Proceed commands from CLI
    m_ipc.value().run([&](const IPC::Command cmd, const std::vector<std::string>& args) {
        return ipc_command_receiver(cmd, args);
    });
}




// Private method
std::string CLI::ipc_command_receiver(const IPC::Command              cmd,
                                      const std::vector<std::string>& args) {
    // --list <args>
    if (cmd == IPC::Command::LIST) {
        // --list modules
        if (args[0] == "modules") {
            return list_modules();
        }
    }


    // --run <args>
    if (cmd == IPC::Command::RUN) {
        // --run <modules>
        for (const auto& module_name : args) {

            if (auto iter = std::find_if(
                    m_app.m_modules.begin(),
                    m_app.m_modules.end(),
                    [&](const auto& module) { return module->module_name() == module_name; });
                iter != m_app.m_modules.end()) {

                // If found module with name `module_name`
                (*iter)->enable();
                return "\033[32mDone!\033[0m";

            } else {
                // Return red error
                return std::format("\033[31mModule with name {} doesn't exists!\033[0m",
                                   module_name);
            }
        }
    }


    // --stop <args>
    if (cmd == IPC::Command::STOP) {
        // --run <modules>
        for (const auto& module_name : args) {

            if (auto iter = std::find_if(
                    m_app.m_modules.begin(),
                    m_app.m_modules.end(),
                    [&](const auto& module) { return module->module_name() == module_name; });
                iter != m_app.m_modules.end()) {

                // If found module with name `module_name`
                (*iter)->disable();
                return "\033[32mDone!\033[0m";

            } else {
                // Return red error
                return std::format("\033[31mModule with name {} doesn't exists!\033[0m",
                                   module_name);
            }
        }
    }

    return "Invalid syntax";
}




// Private method
std::string CLI::list_modules() const {
    std::string result = "NAME\t\tSTATUS\t\tDESCRIPTION\n";

    for (const auto& module : m_app.m_modules) {
        std::string current_module_info(1, '\n');

        // Module name
        current_module_info.append(module->module_name());

        // Tab
        current_module_info.append("\t\t");

        // Status
        if (module->is_enabled()) {
            // Print green module name
            current_module_info.append("\033[32mRUNNING\033[0m");
        } else {
            // Print red module name
            current_module_info.append("\033[31mSTOPPED\033[0m");
        }

        // Tab
        current_module_info.append("\t\t");

        // Description
        current_module_info.append(module->module_description());

        result.append(current_module_info);
    }

    return result;
}



} // namespace smu_server
