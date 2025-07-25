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

class IPC {
  public:
    /**
     * @brief All kinds of command options
     * @see `run`
     */
    enum class Command { LIST = 0, RUN = 1, STOP = 2 };




    /**
     * @brief IPC constructor
     * @param abstract_socket_name Name of abstract socket to interprocess communication with CLI
     */
    IPC(const std::string_view abstract_socket_name);




    /**
     * @brief Starts receiving a message followed by sending a reply
     * @param callback Callback that is called when a command is received. Accepts a command and a
     * list of arguments. Should return a response that is passed to the client and displayed in the
     * client's terminal
     */
    void run(std::function<std::string(const Command command, const std::vector<std::string>&)>
                 callback);



  private:
    IPC_IO m_ipc_io;
};

} // namespace smu_server
