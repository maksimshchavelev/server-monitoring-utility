/// GPLv3 LICENSE, Copyright (©) 2025, Maksim Shchavelev <maksimshchavelev@gmail.com>
/// See LICENSE for details

/**
 * @file modules/network/network.cpp
 * @brief File with network class to connect to a websocket server
 */

#include "modules/network/network.hpp"
#include "compile-time_config.hpp"
#include "ixwebsocket/IXNetSystem.h"
#include <filesystem>

namespace smu {

// Public constructor
Network::Network(const Settings& settings) : m_settings(settings) {}




// Public method
void Network::run(std::function<void(const std::vector<uint8_t>&)> on_message,
                  std::function<void(const std::string&)> on_connection_error) {

    const std::string path_to_cert =
        std::format("{}certs/{}.crt", CONFIG_ROOT_DIR, m_settings.get_ip());
    // Check if certificate exists
    if (!std::filesystem::exists(path_to_cert)) {
        throw std::runtime_error(std::format(
            "Certificate for IP {} ({}) doesn't exists!", m_settings.get_ip(), path_to_cert));
    }

    ix::initNetSystem(); // For Windows. _WIN32 macro inside

    // Setup TLS options
    ix::SocketTLSOptions tls_options;
    // Specify the trusted certificate
    tls_options.caFile = path_to_cert;

    // Create connection
    m_connection = std::make_unique<ix::WebSocket>();

    m_connection->setUrl(std::format("wss://{}:{}", m_settings.get_ip(), m_settings.get_port()));
    m_connection->setPingInterval(20);
    m_connection->setTLSOptions(tls_options);

    m_connection->setOnMessageCallback(
        [on_message, on_connection_error](const ix::WebSocketMessagePtr& msg) {
            if (msg->type == ix::WebSocketMessageType::Message && msg->binary) {
                const auto& msg_str = msg->str;
                on_message(std::vector<uint8_t>(msg_str.begin(), msg_str.end()));
            }
            // If connection error
            else if (msg->type == ix::WebSocketMessageType::Error) {
                on_connection_error(msg->errorInfo.reason);
            }
        });

    m_connection->start();
}




// Public method
void Network::stop() {
    m_connection->stop();
    ix::uninitNetSystem(); // For Windows. _WIN32 macro inside
}




} // namespace smu
