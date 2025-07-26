/// GPLv3 LICENSE, Copyright (©) 2025, Maksim Shchavelev <maksimshchavelev@gmail.com>
/// See LICENSE for details

/**
 * @file network/network.hpp
 * @brief File with network part of smu-server
 */

#pragma once

#include "controllers/websocket_main_controller.hpp"

namespace smu_server {

/**
 * @brief Class for networking
 */
class Network {
  public:
    /**
     * @brief Default constructor
     */
    Network() = default;




    /**
     * @brief Runs network
     * @port The port on which the server will be launched
     * @param callback Callback that will be called immediately after the server starts
     * @note Blocks thread
     */
    void run(uint16_t port, std::function<void()> callback);




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
    std::shared_ptr<MainWebsocketController> m_main_ws_controller;
};

} // namespace smu_server
