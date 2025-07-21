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
UI::UI() : m_screen(ftxui::ScreenInteractive::TerminalOutput()), m_tabs(std::make_shared<Tabs>()) {}




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
    m_screen.ExitLoopClosure()();
    m_screen.Exit();

    while (m_screen.Active()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}




// Public method
void UI::set_data(const Json::Value& data) {
    std::lock_guard<std::mutex> lock(m_data_mutex);
    m_data = data;

    std::vector<ftxui::Element>   m_tabs_headers;
    std::vector<ftxui::Component> m_tabs_content;

    // Set headers and content
    for (const auto& module_name : m_data.getMemberNames()) {
        m_tabs_headers.push_back(ftxui::text(module_name));
        m_tabs_content.push_back(unwrap_module(m_data[module_name]));
    }

    m_tabs->set_data(std::move(m_tabs_headers), std::move(m_tabs_content));
    m_screen.PostEvent(ftxui::Event::Custom);
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

        auto& name = key;

        if (type == "value") {
            // Value display component
            std::string value = inner_node["value"].asString();
            std::string units = inner_node["units"].asString();

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

            children.push_back(Renderer([=] {
                static bool state{false};
                state = !state;

                auto res =
                    text(std::format("{}: {} {}", name, std::move(value), std::move(units))) |
                    color(text_color);

                if (alarm_flag) {
                    res |= inverted;
                }

                return res;
            }));

        } else {
            // If type is container or root
            Component child = unwrap_module(inner_node);
            Component wrapped = Renderer(child, [name, child] {
                return hbox(
                    {window(text(name) | color(Color::RGB(0, 0, 255)), child->Render()), filler()});
            });
            children.push_back(wrapped);
        }
    }

    return Container::Vertical(std::move(children));
}


} // namespace smu
