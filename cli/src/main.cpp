/// GPLv3 LICENSE, Copyright (©) 2025, Maksim Shchavelev <maksimshchavelev@gmail.com>
/// See LICENSE for details

#include "compile-time_config.hpp"
#include "utils/utils.hpp"
#include <cstddef>
#include <iostream>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netinet/tcp.h>


int main(int argc, char** argv) {
    // If no additional arguments passed (first is working directory)
    if (argc == 1) {
        // Print red error
        std::cout << "\033[31m" << "No arguments were passed on. Exit" << "\033[0m" << std::endl;
        return EXIT_FAILURE;
    } // else

    if (argc > 256) {
        std::cout << "\033[31m" << "Too many arguments. Exit" << "\033[0m" << std::endl;
        return EXIT_FAILURE;
    }


    // Parsing own arguments
    if (smu_cli::parse_own_arguments(argc, argv)) {
        // If own arguments parsed
        return EXIT_SUCCESS;
    }


    // Merging arguments to one std::string
    // Arguments are separated by a space
    std::string merged_arguments;
    merged_arguments.append(argv[1]); // appending first argument
    for (int i = 2; i < argc; ++i) {
        // appending last arguments
        merged_arguments.push_back(' ');
        merged_arguments.append(argv[i]);
    }

    merged_arguments.push_back('\0'); // End char for correctly reading at server


    // Creating socket
    int socket_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (socket_fd == -1) { // error
        smu_cli::handle_error("socket");
    }


    // Filling sockaddr_un
    sockaddr_un addr;
    memset(&addr, 0x0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    addr.sun_path[0] = '\0'; // for abstract socket
    strncpy(addr.sun_path + 1,
            ABSTRACT_SOCKET_NAME,
            sizeof(addr.sun_path) - 1); // -1 for terminating zero at the end

    socklen_t socklen = offsetof(sockaddr_un, sun_path) + 1 + strlen(ABSTRACT_SOCKET_NAME);
    // + 1 for terminating zero at the beginning


    // Connecting
    while (connect(socket_fd, reinterpret_cast<sockaddr*>(&addr), socklen) == -1) {
        if (!(errno == EAGAIN || errno == EINTR)) {
            // if error
            smu_cli::handle_error("connect");
        }
        // Trying to connect again...
    }

    // Sending message
    if (!smu_cli::send_message(socket_fd, merged_arguments.data())) {
        // error
        smu_cli::handle_error("send");
    }


    // No more data will be sent
    if (shutdown(socket_fd, SHUT_WR) == -1) {
        smu_cli::handle_error("shutdown");
    }


    // Receive answer
    if (auto res = smu_cli::read_message(socket_fd, 1000, 0x0); res.has_value()) {
        std::cout << res.value() << std::endl;
    } else {
        std::cout << res.error() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
