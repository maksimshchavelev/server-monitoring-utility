/// GPLv3 LICENSE, Copyright (©) 2025, Maksim Shchavelev <maksimshchavelev@gmail.com>
/// See LICENSE for details

/**
 * @file utils/utils.cpp
 * @brief File with helper functions
 */

#include "utils/utils.hpp"
#include "version.hpp"
#include <cxxopts.hpp>
#include <iostream>
#include <poll.h>
#include <sys/socket.h>

namespace smu_cli {

// ============ parse_own_arguments ============
bool parse_own_arguments(int argc, char** argv) noexcept {
    cxxopts::Options options("smu-cli",
                             "CLI part to manage smu-server included in server-monitoring-utility");

    options.add_options()("version", "Show smu-cli version");
    options.add_options()("h,help", "Show help information");
    options.add_options()("commands", "Show the commands available to query the smu-server");

    cxxopts::ParseResult result;

    try {
        result = options.parse(argc, argv);
    } catch (std::exception& e) {
        return false; // not parsed
    }

    if (result.contains("version")) {
        std::cout << std::format("Version is {}.{}.{}",
                                 PROJECT_VERSION_MAJOR,
                                 PROJECT_VERSION_MINOR,
                                 PROJECT_VERSION_PATCH)
                  << std::endl;
        return true; // parsed
    }

    if (result.contains("help")) {
        std::cout << options.help();
        return true; // parsed
    }

    // ELSE
    return false; // not parsed
}



// ============ handle_error ============
void handle_error(const char* str) noexcept {
    perror(str);
    exit(EXIT_FAILURE);
}




// ============ send_message ============
bool send_message(int socket_fd, const std::string_view str) noexcept {
    size_t  total_bytes = str.size();
    size_t  bytes_sent = 0; // bytes sent
    ssize_t res;

    while ((res = send(socket_fd, str.data() + bytes_sent, total_bytes - bytes_sent, 0)) <
           static_cast<ssize_t>(total_bytes)) {
        // if error
        if (res == -1) {
            if (!(errno == EAGAIN || errno == EINTR)) {
                // if fatal error
                return false;
            }
        }
        // Connection closed
        if (res == 0) {
            return false;
        }

        bytes_sent += static_cast<size_t>(res);
    }

    return true;
}




// ============ read_message ============
std::expected<std::string, std::string> read_message(int        socket_fd,
                                                     int        timeout_ms,
                                                     const char msg_end) noexcept {
    // Poll
    pollfd fd;
    fd.fd = socket_fd;
    fd.events = POLLIN;

    int poll_res;
    while ((poll_res = poll(&fd, 1, timeout_ms)) != 1) {
        // Error
        if (poll_res == -1) {
            // Fatal error
            if (errno != EINTR) {
                return std::unexpected(std::format("poll: {}", strerror(errno)));
            }
            // continue
        }
        // timeout
        else if (poll_res == 0) {
            return std::unexpected("Error: response timeout has been exceeded");
        }
    }

    // Error
    if (fd.revents & (POLLERR | POLLHUP | POLLNVAL)) {
        return std::unexpected("socket error during poll");
    }

    std::string result;
    char        buffer[256];

    ssize_t rd_res{0};

    while (true) {
        rd_res = recv(socket_fd, buffer, sizeof(buffer), 0);

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


} // namespace smu_cli
