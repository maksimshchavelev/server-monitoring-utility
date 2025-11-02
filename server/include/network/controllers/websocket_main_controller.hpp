/// GPLv3 LICENSE, Copyright (©) 2025, Maksim Shchavelev <maksimshchavelev@gmail.com>
/// See LICENSE for details

/**
 * @file network/controllers/websocket_main_controller.hpp
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
 * @brief Main websocket controller for network IO
 *
 * Data is transmitted through this controller. It stores a list of connected clients
 * and has a method for transmitting data to all clients simultaneously
 */
class MainWebsocketController : public drogon::WebSocketController<MainWebsocketController, false> {
  public:
    WS_PATH_LIST_BEGIN

    WS_PATH_ADD("/", drogon::Get);

    WS_PATH_LIST_END



    /**
     * @brief MainWebsocketController constructor
     */
    MainWebsocketController() = default;




    /**
     * @brief Handles new message. Does nothing
     * @see Drogon documentation
     * (https://drogonframework.github.io/drogon-docs/#/ENG/ENG-04-3-Controller-WebSocketController)
     */
    void handleNewMessage(const drogon::WebSocketConnectionPtr& connection,
                          std::string&&                         message,
                          const drogon::WebSocketMessageType&   message_type) override;



    /**
     * @brief Handles new connection. Adds the client to the list of connected clients
     * @see Drogon documentation
     * (https://drogonframework.github.io/drogon-docs/#/ENG/ENG-04-3-Controller-WebSocketController)
     */
    void handleNewConnection(const drogon::HttpRequestPtr&         request,
                             const drogon::WebSocketConnectionPtr& connection) override;




    /**
     * @brief Handles closing connection. Removes the client from the list of connected clients
     * @see Drogon documentation
     * (https://drogonframework.github.io/drogon-docs/#/ENG/ENG-04-3-Controller-WebSocketController)
     */
    void handleConnectionClosed(const drogon::WebSocketConnectionPtr& connection) override;




    /**
     * @brief Sends json data with metrics to all connected clients
     * @param data `std::vector<uint8_t>` with data in MDTP protocol
     */
    void send_everyone(const std::vector<uint8_t>& data);




    /**
     * @brief Returns count of connections
     * @return `std::size_t` with count of connections
     */
    std::size_t get_connections_count() const noexcept;


  private:
    std::set<drogon::WebSocketConnectionPtr> m_connections{}; ///< set with connections
    mutable std::mutex m_connections_mutex{};  ///< to prevent data race with `m_connections`
    std::size_t        m_connections_count{0}; ///< count of connections
};

} // namespace smu_server
