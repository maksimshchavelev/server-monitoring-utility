/// GPLv3 LICENSE, Copyright (©) 2025, Maksim Shchavelev <maksimshchavelev@gmail.com>
/// See LICENSE for details

/**
 * @file ipc/ipc.cpp
 * @brief File with smu-cli interprocessor communication tools
 */

#include "ipc/ipc.hpp"
#include <cxxopts.hpp>

namespace smu_server {

// Public constructor
IPC::IPC(const std::string_view abstract_socket_name) : m_ipc_io(abstract_socket_name) {}




// Public method
void IPC::run(
    std::function<std::string(const Command, const std::vector<std::string_view>&)> callback) {
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
        cxxopts::Options options("smu-server");
        options.add_options()(
            "list", "List sub-entities (modules/configs/etc)", cxxopts::value<std::string>());

        auto tokens = split_like_shell(str);

        // `const char*` view for options.parse
        std::vector<const char*> args;

        for (const auto& token : tokens) {
            args.push_back(token.data());
        }

        options.parse_positional({ "list" });

        auto result = options.parse(static_cast<int>(args.size()), args.data());

        // If have list command
        if(result.contains("list")) {
            const std::vector<std::string_view> args = { result["list"].as<std::string>() };
            return callback(Command::LIST, args);
        }

        return "Unknown command";
    };

    // Running
    m_ipc_io.run_listening_async(cb);
}




} // namespace smu_server
