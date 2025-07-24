/// GPLv3 LICENSE, Copyright (©) 2025, Maksim Shchavelev <maksimshchavelev@gmail.com>
/// See LICENSE for details

/**
 * @file modules/ui/ui.cpp
 * @brief File with user interface
 */


#include "modules/ui/ui.hpp"
#include <ftxui/component/component.hpp>
#include <iostream>
#include <thread>


namespace smu {


// Public constructor
UI::UI() : m_screen(ftxui::ScreenInteractive::TerminalOutput()), m_tabs(make_tabs()) {}




// Public method
void UI::run_async() {
    std::thread runner([&]() {
        using namespace ftxui;

#if defined(__unix__)
        std::cout << "\033[2J\033[H"; // ANSI code for clear screen in Linux
#elif defined(_WIN32) or defined(_WIN64)
        std::cout << "\x1B[2J\x1B[H"; // ANSI code for clear screen in Windows
#endif

        m_screen.Loop(m_tabs);
    });
    runner.detach();
}




// Public method
void UI::stop() {
    m_screen.Exit();

    while (m_screen.Active()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}




// Public method
void UI::set_data(const Json::Value& data) {
    Json::Value data_copy = data;

    m_screen.Post([this, data_copy = std::move(data_copy)]{
        std::lock_guard<std::mutex> lock(m_data_mutex);
        m_data = std::move(data_copy);

        std::vector<ftxui::Element>   m_tabs_headers;
        std::vector<ftxui::Component> m_tabs_content;

        // Set headers and content
        for (const auto& module_name : m_data.getMemberNames()) {
            m_tabs_headers.push_back(ftxui::text(module_name));
            m_tabs_content.push_back(unwrap_module(m_data[module_name]));
        }

        m_tabs->set_data(std::move(m_tabs_headers), std::move(m_tabs_content));
        m_screen.PostEvent(ftxui::Event::Custom);
    });
}




// Private method
ftxui::Component UI::unwrap_module(const Json::Value& data) {
    using namespace ftxui;

    // Return error if data is empty
    if (data.empty()) {
        return Renderer([] { return text("Nothing to draw!"); });
    }

    // vector with components
    std::vector<Component> children;
    children.reserve(data.size());


    for (const auto& key : data.getMemberNames()) {
        if (key == "type")
            continue;

        const Json::Value& inner_node = data[key];
        const std::string  type = inner_node["type"].asString();

        if (type == "value") {
            // Value display component
            std::string value = inner_node["value"].asString();
            std::string units = inner_node["units"].asString();

            // Display N/A if value is empty
            if (value.empty()) {
                value = "N/A";
            }

            Color text_color(Color::RGB(255, 255, 255));
            bool  alarm_flag{false};

            if (units == "%") {
                int number_value = std::stoi(value);

                // Yellow zone
                if (number_value >= 40 && number_value < 70) {
                    text_color = Color::RGB(255, 255, 0);
                    alarm_flag = true;
                }
                // Red zone
                else if (number_value >= 70 && number_value <= 100) {
                    text_color = Color::RGB(255, 0, 0);
                    alarm_flag = true;
                }
            }

            children.push_back(Renderer([keyname_copy = std::string(key),
                                         value_copy = std::string(value),
                                         units_copy = std::string(units),
                                         alarm_flag_copy = alarm_flag,
                                         color_copy = text_color] {

                auto res =
                    text(std::format(
                        "{}: {} {}", keyname_copy, std::move(value_copy), std::move(units_copy))) |
                    color(color_copy);

                if (alarm_flag_copy) {
                    res |= inverted;
                }

                return res;
            }));

        } else {
            // If type is container or root
            Component child = unwrap_module(inner_node);
            Component wrapped = Renderer(child, [keyname_copy = std::string(key), child] {
                return hbox(
                    {window(text(keyname_copy) | color(Color::RGB(0, 0, 255)), child->Render()), filler()});
            });
            children.push_back(wrapped);
        }
    }

    return Container::Vertical(std::move(children));
}


} // namespace smu
