/// GPLv3 LICENSE, Copyright (©) 2025, Maksim Shchavelev <maksimshchavelev@gmail.com>
/// See LICENSE for details

/**
 * @file core/controllers/websocket_main_controller.cpp
 * @brief Main websocket controller implementation
 */


#include "core/controllers/websocket_main_controller.hpp"


namespace smu_server {


// Public method
void MainWebsocketController::handleNewMessage(const drogon::WebSocketConnectionPtr&,
                                               std::string&&,
                                               const drogon::WebSocketMessageType&) {}




// Public method
void MainWebsocketController::handleNewConnection(
    const drogon::HttpRequestPtr&, const drogon::WebSocketConnectionPtr& connection) {
    m_connections.insert(connection);
}




// Public method
void MainWebsocketController::handleConnectionClosed(
    const drogon::WebSocketConnectionPtr& connection) {
    m_connections.erase(connection);
}




// Public method
void MainWebsocketController::send_everyone(const Json::Value& data) {
    for (auto& connection : m_connections) {
        connection->sendJson(data);
    }
}




// Public method
std::size_t MainWebsocketController::get_connections_count() const noexcept {
    return m_connections.size();
}



} // namespace smu_server
