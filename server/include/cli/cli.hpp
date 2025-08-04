/// GPLv3 LICENSE, Copyright (©) 2025, Maksim Shchavelev <maksimshchavelev@gmail.com>
/// See LICENSE for details

/**
 * @file cli/cli.hpp
 * @brief File with smu-cli command receiver
 */

#pragma once

#include "ipc/ipc.hpp"
#include <optional>

namespace smu_server {

class Application; // Forward declaration to avoid cyclic including

/**
 * @brief The CLI class to processing CLI commands
 *
 * This class has full access to `Application` and its internal methods and fields. It
 * autonomously processes incoming commands and returns a response. It uses the `IPC` class for
 * interprocess communication with the `smu-cli` utility.
 */
class CLI {
  public:
    /**
     * @brief CLI default construcor. CLI has access to all Application fields and methods
     * @param app Reference to `Application`
     * @note Command listening does not start automatically. Call `CLI::run` for this.
     */
    CLI(Application& app);




    /**
     * @brief Runs CLI async
     */
    void run();


  private:
    Application&       m_app; ///< Reference to Applicaiton instance
    std::optional<IPC> m_ipc; ///< Optional for lazy initialization in `run_async`


    /**
     * @brief Receives commands from the IPC and processes them. Passed to the `IPC::run` callback
     * @param cmd Command type
     * @param args Command args
     * @return The response to the command, which is then passed to smu-cli
     */
    std::string ipc_command_receiver(const IPC::Command cmd, const std::vector<std::string>& args);


    /**
     * @brief Get modules name, status and description
     * @return `std::string`
     */
    std::string list_modules() const;
};

} // namespace smu_server
