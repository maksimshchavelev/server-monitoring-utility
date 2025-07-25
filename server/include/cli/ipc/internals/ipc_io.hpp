/// GPLv3 LICENSE, Copyright (©) 2025, Maksim Shchavelev <maksimshchavelev@gmail.com>
/// See LICENSE for details

/**
 * @file ipc/internals/ipc_io.hpp
 * @brief File with async IPC Input-Output class to send/receive raw messages
 */

#pragma once

#include <expected>
#include <functional>
#include <string>
#include <string_view>

namespace smu_server {

/**
 * @brief The class with async IPC Input-Output class to send/receive raw messages
 */
class IPC_IO {
  public:
    /**
     * @brief IPC_IO constructor
     * @param abstract_socket_name Name of abstract socket to interprocess communication with CLI
     */
    IPC_IO(const std::string_view abstract_socket_name);




    /**
     * @brief Starts async message receiving and sending a reply back in a separate thread
     * @param callback Function to be called when the message is received. The function should
     * return a message to be sent to the client
     * @note Can be runned only once
     */
    void run_listening_async(std::function<std::string(const std::string_view message)> callback);




  private:
    const std::string m_abstract_socket_name;
    int               m_socket_fd{0}; // File descriptor of socket. Configuring in init()




    /**
     * @brief Configures the socket
     * @return `std::expected` with `void` if success, otherwise with `std::string` with error
     * description
     */
    std::expected<void, std::string> init() noexcept;




    /**
     * @brief Reads message from socket
     * @param connected_socket_fd Socket received via `accept`.
     * @param msg_end End of message marker
     * @return `std::expected` with `std::string` with data if success, otherwise with `std::string`
     * with error
     */
    std::expected<std::string, std::string> receive_message(int        connected_socket_fd,
                                                            const char msg_end = '\n') noexcept;




    /**
     * @brief Sends a message to an open `socket_fd` in one call
     * @param socket_fd File descriptor of an open socket
     * @param str The string that will be sent
     * @return `std::expected` with `void` if success, otherwise with `std::string` with error
     */
    [[nodiscard("The result of `send_message` cannot be ignored")]] std::expected<void, std::string>
    send_message(int socket_fd, const std::string_view str) noexcept;
};

} // namespace smu_server
