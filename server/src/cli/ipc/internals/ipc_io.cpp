/// GPLv3 LICENSE, Copyright (©) 2025, Maksim Shchavelev <maksimshchavelev@gmail.com>
/// See LICENSE for details

/**
 * @file ipc/internals/ipc_io.cpp
 * @brief File with async IPC Input-Output class to send/receive_message raw messages
 */

#include "cli/ipc/internals/ipc_io.hpp"
#include <format>
#include <sys/socket.h>
#include <sys/un.h>
#include <thread>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wold-style-cast"
#include "trantor/utils/Logger.h"
#pragma GCC diagnostic pop

namespace smu_server {

// Public constructor
IPC_IO::IPC_IO(const std::string_view abstract_socket_name) :
    m_abstract_socket_name(abstract_socket_name) {

    // If socket initialization error
    if (auto res = init(); !res.has_value()) {
        std::string error_string =
            std::format("Can't init abstract socket with name, cause: {}", res.error());
        LOG_ERROR << error_string;
        throw std::runtime_error(error_string);
    }
}




// Public method
void IPC_IO::run_listening_async(std::function<std::string(const std::string_view)> callback) {
    static bool running{false};

    if (running) {
        throw std::runtime_error("Double running of IPC_IO::run_listening_async");
    }

    running = true;

    std::thread runner([this, callback]() {
        while (true) {

            sockaddr_un peer_addr;
            socklen_t   peer_addr_size;
            int         connection_fd;

            while (true) {
                connection_fd =
                    accept(m_socket_fd, reinterpret_cast<sockaddr*>(&peer_addr), &peer_addr_size);
                if (connection_fd == -1) {
                    // If fatal error
                    if (errno != EINTR) {
                        LOG_WARN << "IPC_IO: error accepting connection, cause: "
                                 << strerror(errno);
                        break;
                    }
                } else {
                    break;
                }
            }

            // If broken descriptor
            if (connection_fd == -1) {
                continue;
            }

            // Reading message
            auto received_res = receive_message(connection_fd);
            if (!received_res.has_value()) {
                // If error
                LOG_WARN << "Can't read message from socket " << connection_fd
                         << ", cause: " << received_res.error();
                close(connection_fd);
                continue;
            }

            // If success
            std::string msg_to_send;

            try {
                msg_to_send = callback(received_res.value());
            } catch (std::exception& e) {
                std::string error_message =
                    std::format("Error parsing arguments, cause: {}", e.what());
                error_message.push_back('\0');
                LOG_WARN << error_message;

                auto send_res = send_message(connection_fd, error_message);
                if (!send_res.has_value()) {
                    // If error
                    LOG_WARN << "Can't send message to socket " << connection_fd
                             << ", cause: " << send_res.error();
                    close(connection_fd);
                    continue;
                }

                close(connection_fd);
                continue;
            }

            msg_to_send.push_back('\0');

            // Sending
            auto send_res = send_message(connection_fd, msg_to_send);
            if (!send_res.has_value()) {
                // If error
                LOG_WARN << "Can't send message to socket " << connection_fd
                         << ", cause: " << send_res.error();
                close(connection_fd);
                continue;
            }

            close(connection_fd);
        }
    });

    runner.detach();
}




// Private method
std::expected<void, std::string> IPC_IO::init() noexcept {
    m_socket_fd = socket(AF_UNIX, SOCK_STREAM, 0);

    // Error
    if (m_socket_fd == -1) {
        m_socket_fd = 0; // reset m_socket_fd
        return std::unexpected(std::format("error create abstract socket with name {}: {}",
                                           m_abstract_socket_name,
                                           strerror(errno)));
    }

    // Configuring sockaddr_un
    sockaddr_un addr;
    memset(&addr, 0x0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    addr.sun_path[0] = '\0'; // for abstract socket

    // Avoid sun_path overflow
    if(m_abstract_socket_name.size() > sizeof(addr.sun_path) - 1) {
        return std::unexpected("Socket name too long");
    }

    // Subtract 1 because the first byte is occupied by the character ‘\0’.
    memcpy(addr.sun_path + 1, m_abstract_socket_name.data(), sizeof(addr.sun_path) - 1);

    // We add 1 because the first byte is occupied by the character ‘\0’.
    // We count the socket length as the size of sun_family plus the length of the name along with
    // the ‘\0’ character.
    // However, you should use offsetof because the compiler may add padding between the two fields
    // This way we will calculate the size of sun_family taking into account possible alignment,
    // while we already know the size of sun_path
    socklen_t socklen = offsetof(sockaddr_un, sun_path) + 1 +
                        static_cast<unsigned int>(m_abstract_socket_name.length());

    // Binding socket
    if (bind(m_socket_fd, reinterpret_cast<sockaddr*>(&addr), socklen) == -1) {
        // Error
        return std::unexpected(std::format("error bind abstract socket with name {}: {}",
                                           m_abstract_socket_name,
                                           strerror(errno)));
    }

    // Opening for listening
    if (listen(m_socket_fd, 1) == -1) {
        return std::unexpected(std::format("error setup listen abstract socket with name {}: {}",
                                           m_abstract_socket_name,
                                           strerror(errno)));
    }

    return {}; // success
}




// Private method
std::expected<std::string, std::string> IPC_IO::receive_message(int        connected_socket_fd,
                                                                const char msg_end) noexcept {
    std::string result;
    char        buffer[256];

    while (true) {
        ssize_t rd_res = recv(connected_socket_fd, buffer, sizeof(buffer), 0);

        // Error occured
        if (rd_res == -1) {
            if (!(errno == EAGAIN || errno == EINTR)) {
                // Fatal error
                return std::unexpected(std::format("recv: {}", strerror(errno)));
            }
            // continue
        } else if (rd_res == 0) {
            // Connection closed
            break;
        } // else

        result.append(buffer, static_cast<size_t>(rd_res));

        if (auto pos = result.find(msg_end); pos != std::string::npos) {
            result.resize(pos);
            break;
        }
    }

    return result;
}




// Private method
std::expected<void, std::string> IPC_IO::send_message(int                    socket_fd,
                                                      const std::string_view str) noexcept {
    size_t  total_bytes = str.size();
    size_t  bytes_sent = 0; // bytes sent
    ssize_t res;

    while ((res = send(socket_fd, str.data() + bytes_sent, total_bytes - bytes_sent, 0)) <
           static_cast<ssize_t>(total_bytes)) {
        // if error
        if (res == -1) {
            if (!(errno == EAGAIN || errno == EINTR)) {
                // if fatal error
                return std::unexpected(strerror(errno));
            }
        }
        // Connection closed
        if (res == 0) {
            return std::unexpected(strerror(errno));
        }

        bytes_sent += static_cast<size_t>(res);
    }

    return {};
}

} // namespace smu_server
