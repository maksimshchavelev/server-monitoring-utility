/// GPLv3 LICENSE, Copyright (©) 2025, Maksim Shchavelev <maksimshchavelev@gmail.com>
/// See LICENSE for details

/**
 * @file core/controllers/websocket_main_controller.hpp
 * @brief Main websocket controller
 */

#pragma once

// We need this to remove compilation warnings inside Drogon
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wuseless-cast"
#include <drogon/WebSocketController.h>
#pragma GCC diagnostic pop


#include <set>

namespace smu_server {

/**
 * @brief Main websocket controller
 */
class MainWebsocketController : public drogon::WebSocketController<MainWebsocketController, false> {
  public:
    WS_PATH_LIST_BEGIN

    WS_PATH_ADD("/get_metrics", drogon::Get);

    WS_PATH_LIST_END



    /**
     * @brief MainWebsocketController constructor
     */
    MainWebsocketController() = default;




    /**
     * @brief Handles new message.
     * @see Drogon documentation
     * (https://drogonframework.github.io/drogon-docs/#/ENG/ENG-04-3-Controller-WebSocketController)
     */
    void handleNewMessage(const drogon::WebSocketConnectionPtr& connection,
                          std::string&&                         message,
                          const drogon::WebSocketMessageType&   message_type) override;



    /**
     * @brief Handles new connection.
     * @see Drogon documentation
     * (https://drogonframework.github.io/drogon-docs/#/ENG/ENG-04-3-Controller-WebSocketController)
     */
    void handleNewConnection(const drogon::HttpRequestPtr&         request,
                             const drogon::WebSocketConnectionPtr& connection) override;




    /**
     * @brief Handles closing connection.
     * @see Drogon documentation
     * (https://drogonframework.github.io/drogon-docs/#/ENG/ENG-04-3-Controller-WebSocketController)
     */
    void handleConnectionClosed(const drogon::WebSocketConnectionPtr& connection) override;




    /**
     * @brief Sends json data with metrics to all clients
     * @param data `Json::Value&` with data
     */
    void send_everyone(const Json::Value& data);




    /**
     * @brief Returns count of connections
     * @return `std::size_t` with count of connections
     */
    std::size_t get_connections_count() const noexcept;


  private:
    std::set<drogon::WebSocketConnectionPtr> m_connections{};
    mutable std::mutex m_connections_mutex{};
};

} // namespace smu_server
