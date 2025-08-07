/// GPLv3 LICENSE, Copyright (©) 2025, Maksim Shchavelev <maksimshchavelev@gmail.com>
/// See LICENSE for details

/**
 * @file modules/network/network.cpp
 * @brief File with network class to connect to a websocket server
 */

#include "modules/network/network.hpp"
#include "ixwebsocket/IXNetSystem.h"

namespace smu {

// Public constructor
Network::Network(const Settings& settings) : m_settings(settings) {}




// Public method
void Network::run(std::function<void(const std::string&)> on_message,
                  std::function<void(const std::string&)> on_connection_error) {

    ix::initNetSystem(); // For Windows. _WIN32 macro inside

    m_connection = std::make_unique<ix::WebSocket>();

    m_connection->setUrl(std::format("ws://{}:{}", m_settings.get_ip(), m_settings.get_port()));
    m_connection->setPingInterval(20);

    m_connection->setOnMessageCallback([on_message, on_connection_error](const ix::WebSocketMessagePtr& msg) {
        if (msg->type == ix::WebSocketMessageType::Message) {
            on_message(msg->str);
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
