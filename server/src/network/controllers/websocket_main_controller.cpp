/// GPLv3 LICENSE, Copyright (©) 2025, Maksim Shchavelev <maksimshchavelev@gmail.com>
/// See LICENSE for details

/**
 * @file network/controllers/websocket_main_controller.cpp
 * @brief Main websocket controller implementation
 */


#include "network/controllers/websocket_main_controller.hpp"


namespace smu_server {


// Public method
void MainWebsocketController::handleNewMessage(const drogon::WebSocketConnectionPtr&,
                                               std::string&&,
                                               const drogon::WebSocketMessageType&) {}




// Public method
void MainWebsocketController::handleNewConnection(
    const drogon::HttpRequestPtr&, const drogon::WebSocketConnectionPtr& connection) {
    std::lock_guard<std::mutex> lock(m_connections_mutex);
    m_connections.insert(connection);
    m_connections_count = m_connections.size();
}




// Public method
void MainWebsocketController::handleConnectionClosed(
    const drogon::WebSocketConnectionPtr& connection) {
    std::lock_guard<std::mutex> lock(m_connections_mutex);
    m_connections.erase(connection);
    m_connections_count = m_connections.size();
}




// Public method
void MainWebsocketController::send_everyone(const std::vector<uint8_t>& data) {
    for (auto& connection : m_connections) {
        connection->send(std::string(data.begin(), data.end()),
                         drogon::WebSocketMessageType::Binary);
    }
}




// Public method
std::size_t MainWebsocketController::get_connections_count() const noexcept {
    return m_connections_count;
}



} // namespace smu_server
