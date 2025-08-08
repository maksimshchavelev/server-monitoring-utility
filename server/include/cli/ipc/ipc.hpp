/// GPLv3 LICENSE, Copyright (©) 2025, Maksim Shchavelev <maksimshchavelev@gmail.com>
/// See LICENSE for details

/**
 * @file ipc/ipc.hpp
 * @brief File with smu-cli interprocessor communication tools
 */

#pragma once

#include "internals/ipc_io.hpp"
#include <functional>
#include <vector>

namespace smu_server {

/**
 * @brief The IPC class to obtain CLI commands via interprocess communication
 */
class IPC {
  public:
    /**
     * @brief All kinds of command options
     * @see `run`
     */
    enum class Command {
        LIST = 0, ///< List something. For example, `--list modules`
        RUN = 1,  ///< Run module. For example, `--run RAM`
        STOP = 2  ///< Stop module. For example, `--stop RAM`
    };




    /**
     * @brief IPC constructor
     * @param abstract_socket_name Name of abstract socket to interprocess communication with CLI
     * @note Message listening does not start automatically. Call `IPC::run`
     */
    IPC(const std::string_view abstract_socket_name);




    /**
     * @brief Starts receiving a messages async followed by sending a reply
     *
     * @param callback Callback that is called when a command is received. Accepts a command and a
     * list of arguments. **Must return a response that is passed to the client and displayed in the
     * client's terminal**
     *
     * @section example_usage Example usage
     *
     * Let's assume that the `IPC` object has already been created and is called `ipc`, and that the
     * following code exists:
     *
     * @code{.cpp}
     * ipc.run([](const Command command, const std::vector<std::string>& args) {
     *      std::cout << "Got command with number " << static_cast<int>(command) << std::endl;
     *      for (const auto& arg : args) {
     *          std::cout << "ARG: " << arg << std::endl;
     *      }
     *
     *      return "this is test answer"; // we must return answer to a command
     * });
     * @endcode
     *
     * Let's assume that we enter the command `--list modules` in **smu-cli**, then on the
     * **smu-server** side we will see:
     *
     * ```{.bash}
     * Got command with number 0
     * ARG: modules
     * ```
     *
     * And the output in **smu-cli** will be as follows:
     *
     * ```{.bash}
     * this is test answer
     * ```
     */
    void run(std::function<std::string(const Command command, const std::vector<std::string>& args)>
                 callback);



  private:
    IPC_IO m_ipc_io; ///< `IPC_IO` object to control IO
};

} // namespace smu_server
