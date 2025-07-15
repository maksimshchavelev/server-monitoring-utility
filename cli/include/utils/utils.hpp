/// GPLv3 LICENSE, Copyright (©) 2025, Maksim Shchavelev <maksimshchavelev@gmail.com>
/// See LICENSE for details

/**
 * @file utils/utils.hpp
 * @brief File with helper functions
 */

#pragma once

#include <expected>
#include <string_view>

namespace smu_cli {

/**
 * @brief Parses arguments intended for itself
 * @param argc argc
 * @param argv argv
 * @return `true` if the arguments were found and the program should be terminated,
 * otherwise `false`
 * @note If invalid arguments are found, the function will return `false` because
 * they could not be parsed
 */
[[nodiscard("The result of `parse_own_options` cannot be ignored")]] bool parse_own_arguments(
    int argc, char** argv) noexcept;




/**
 * @brief Calls `perror(str)` and exits with EXIT_FAILURE
 * @param str String passed to `perror`
 */
void handle_error(const char* str) noexcept;




/**
 * @brief Sends a message to an open `socket_fd` in one call
 * @param socket_fd File descriptor of an open socket
 * @param str The string that will be sent
 * @return `true` if successful, otherwise `false` (see `errno`)
 */
[[nodiscard("The result of `send_message` cannot be ignored")]] bool send_message(
    int socket_fd, const std::string_view str) noexcept;




/**
 * @brief Reads from a socket, you can set a timeout to wait for data
 * @param socket_fd Socket file descriptor
 * @param timeout_ms Timeout in milliseconds
 * @msg_end Message end marker
 * @return `std::expected` with the read data, otherwise the error description
 */
std::expected<std::string, std::string> read_message(int        socket_fd,
                                                     int        timeout_ms,
                                                     const char msg_end = 0x0) noexcept;


} // namespace smu_cli
