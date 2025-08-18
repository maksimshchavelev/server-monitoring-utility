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
     * @param settings `Settings` object. Settings are needed, for example, to specify the port.
     */
    Network(const Settings& settings);




    /**
     * @brief Runs module
     * @param on_message Callback that is called when data is received from
     * the server and has the signature `void(const std::vector<uint8_t>&)`. Bytes are encoded in
     * MDTP protocol
     * @param on_connection_error Callback that is called when a connection error occurs. The reason
     * message is passed to it
     */
    void run(std::function<void(const std::vector<uint8_t>&)> on_message,
             std::function<void(const std::string&)>          on_connection_error);




    /**
     * @brief Aborts connection
     */
    void stop();

  private:
    Settings                       m_settings;   ///< Settings
    std::unique_ptr<ix::WebSocket> m_connection; ///< Connection
};


} // namespace smu
