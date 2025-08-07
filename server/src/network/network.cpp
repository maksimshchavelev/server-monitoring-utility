/// GPLv3 LICENSE, Copyright (©) 2025, Maksim Shchavelev <maksimshchavelev@gmail.com>
/// See LICENSE for details

/**
 * @file network/network.cpp
 * @brief File with network part of smu-server
 */

#include "network/network.hpp"
#include "compile-time_config.hpp"

namespace smu_server {


// Public method
void Network::run(uint16_t port, std::function<void()> callback) {
    // Init controller
    m_main_ws_controller = std::make_shared<MainWebsocketController>();

    drogon::app()
        .addListener("0.0.0.0",
                     port,
                     true,
                     std::format("{}/{}", CONFIGS_DIR, "certificate.crt"),
                     std::format("{}/{}", CONFIGS_DIR, "privkey.key"))
        .registerController(m_main_ws_controller);
    drogon::app().getLoop()->runAfter(0.0, [callback]() { callback(); });
    drogon::app().run();
}




// Public method
void Network::send_everyone(const Json::Value& data) {
    m_main_ws_controller->send_everyone(data);
}




// Public method
std::size_t Network::get_connections_count() const noexcept {
    return m_main_ws_controller->get_connections_count();
}



} // namespace smu_server
