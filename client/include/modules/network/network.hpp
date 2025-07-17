/// GPLv3 LICENSE, Copyright (©) 2025, Maksim Shchavelev <maksimshchavelev@gmail.com>
/// See LICENSE for details

/**
 * @file modules/network/network.hpp
 * @brief File with network class to connect to a websocket server
 */

#pragma once

#include "modules/settings/settings.hpp"
#include <functional>
#include <ixwebsocket/IXWebSocket.h>

namespace smu {

/**
 * @brief Network class to connect to a websocket server
 */
class Network {
  public:
    /**
     * @brief Network constructor
     * @param settings smu::Settings object
     */
    Network(const Settings& settings);




    /**
     * @brief Runs module
     * @param on_message Callback that is called when data is received from
     * the server and has the signature `void (const std::string&)`
     */
    void run(std::function<void(const std::string&)> on_message);




    /**
     * @brief Aborts connetcion
     */
    void stop();

  private:
    Settings m_settings;

    std::unique_ptr<ix::WebSocket> m_connection;
};


} // namespace smu
