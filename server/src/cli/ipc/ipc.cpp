/// GPLv3 LICENSE, Copyright (©) 2025, Maksim Shchavelev <maksimshchavelev@gmail.com>
/// See LICENSE for details

/**
 * @file ipc/ipc.cpp
 * @brief File with smu-cli interprocessor communication tools
 */

#include "cli/ipc/ipc.hpp"
#include <cxxopts.hpp>

namespace smu_server {

// Public constructor
IPC::IPC(const std::string_view abstract_socket_name) : m_ipc_io(abstract_socket_name) {}




// Public method
void IPC::run(std::function<std::string(const Command, const std::vector<std::string>&)> callback) {
    // Lambda for splitting str by ' ' like shell arguments
    auto split_like_shell = [&](const std::string_view str) {
        std::istringstream       iss(str.data());
        std::vector<std::string> args;
        std::string              token;

        while (iss >> std::quoted(token)) {
            args.push_back(token.data());
        }

        return args;
    };


    // Callback for IPC_IO::run_listening_async
    auto cb = [split_like_shell, callback](const std::string_view str) -> std::string {
        cxxopts::Options options("smu-cli");



        options.add_options()
            ("run",  "Run module(s). For example: --run RAM CPU",  cxxopts::value<std::vector<std::string>>())
            ("list", "List sub-entities (--list modules, --list commands)", cxxopts::value<std::string>())
            ("stop", "Stop module(s). For example: --stop CPU", cxxopts::value<std::vector<std::string>>());



        // Split string by ' ' and return vector
        auto tokens = split_like_shell(str);

        // `const char*` view for options.parse
        std::vector<const char*> args;

        args.push_back("fake"); // add fake program name for correct parsing

        for (const auto& token : tokens) {
            args.push_back(token.c_str());
        }


        // Parsing
        auto result = options.parse(static_cast<int>(args.size()), args.data());



        // If have list command
        if (result.contains("list")) {
            // If --list commands
            if(result["list"].as<std::string>() == "commands") {
                return options.help();
            }
            // else
            return callback(Command::LIST, {result["list"].as<std::string>()});
        }

        // If have run command
        else if (result.contains("run")) {
            return callback(Command::RUN, result["run"].as<std::vector<std::string>>());
        }

        // If have stop command
        else if (result.contains("stop")) {
            return callback(Command::STOP, result["stop"].as<std::vector<std::string>>());
        }




        // Default case
        return "Unknown command";
    };

    // Running
    m_ipc_io.run_listening_async(cb);
}




} // namespace smu_server
