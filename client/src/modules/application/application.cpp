/// GPLv3 LICENSE, Copyright (©) 2025, Maksim Shchavelev <maksimshchavelev@gmail.com>
/// See LICENSE for details

/**
 * @file modules/application/application.cpp
 * @brief File with application class to manage other modules
 */

#include "modules/application/application.hpp"
#include <iostream>

namespace smu {

// Public constructor
Application::Application(Settings& settings) : m_settings(settings), m_network(m_settings) {}




// Public method
int Application::run() {
    // Running network
    m_network.run(
        [this](const std::string& msg) {
            if (auto res = json_from_string(msg); res.has_value()) {
                // If no error
                m_ui.set_data(res.value());
            }
        },
        [this](const std::string& connection_error_reason) {
            exit("Connection error, reason: " + connection_error_reason);
        });

    // Running UI
    m_ui.run_async();


    // Waiting for exit signal
    std::mutex mutex;

    {
        std::unique_lock<std::mutex> lock(mutex);
        cw.wait(lock, [this]() { return m_exit_request.load(); });
    }

    return m_return_value.load();
}




// Public method
void Application::exit(std::optional<std::string> error) {
    std::thread([this, error = std::move(error)]{
        m_ui.stop();
        m_network.stop();

        if(error.has_value()) {
            m_return_value.store(1); // error code
            std::cout << error.value() << std::endl; // print error
        } else {
            m_return_value.store(0); // success code
        }

        m_exit_request.store(true);
        cw.notify_one();
    }).detach();
}




// Private method
std::expected<Json::Value, std::string> Application::json_from_string(
    const std::string& str) const noexcept {
    const auto     raw_json_length = static_cast<int>(str.length());
    JSONCPP_STRING err;
    Json::Value    json;

    Json::CharReaderBuilder                 builder;
    const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
    if (!reader->parse(str.c_str(), str.c_str() + raw_json_length, &json, &err)) {
        // error
        return std::unexpected(std::format("Error conversion to json: {}", err));
    }

    return json;
}




} // namespace smu
