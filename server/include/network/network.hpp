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
     *
     * @param port The port on which the server will be launched
     * @param callback Callback that will be called immediately after the server starts. For
     * example, to perform an action or start a service immediately after starting the server:
     * @code{.cpp}
     * // network - Network class object
     * // We running server at 5050 port
     * network.run(5050, []() {
     *      // This is where the asynchronous launch of metrics distribution to clients takes place
     *      run_sending_metrics_async();
     * });
     * @endcode
     *
     * @note Blocks thread. Run in a separate thread
     */
    void run(uint16_t port, std::function<void()> callback);




    /**
     * @brief Sends json data with metrics to all clients
     * @param data `std::vector<uint8_t>` with data (MDTP protocol)
     * @note Call after calling `Network::run`
     */
    void send_everyone(const std::vector<uint8_t>& data);




    /**
     * @brief Returns count of connections
     * @return `std::size_t` with count of connections
     * @note Call after calling `Network::run`
     */
    std::size_t get_connections_count() const noexcept;



  private:
    std::shared_ptr<MainWebsocketController>
        m_main_ws_controller; ///< `MainWebsocketController` shared ptr
};

} // namespace smu_server
