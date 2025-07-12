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
UI::UI() : m_screen(ftxui::ScreenInteractive::TerminalOutput()) {}




// Public method
void UI::run_async() {
    std::thread runner([this]() {
        using namespace ftxui;

        auto renderer = Renderer([&] {
            return unwrap_module(m_data["RAM"])->Render();
        });

        m_screen.Loop(renderer);
    });
    runner.detach();
}




// Public method
void UI::stop() {
    m_screen.ExitLoopClosure()();
    m_screen.Exit();
    m_screen.RequestAnimationFrame();
    m_screen.PostEvent(ftxui::Event::Custom);
}




// Public method
void UI::set_data(const Json::Value& data) {
    m_data = data;
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
        if (key == "type") continue;

        const Json::Value& inner_node = data[key];
        const std::string type = inner_node["type"].asString();

        auto& name = key;

        if (type == "value") {
            // Value display component
            std::string value = inner_node["value"].asString();
            std::string units = inner_node["units"].asString();

            children.push_back(Renderer([=] {
                return text(std::format("{}: {} {}", name, std::move(value), std::move(units)));
            }));

        } else {
            // If type is container or root
            Component child = unwrap_module(inner_node);
            Component wrapped = Renderer(child, [name, child] {
                return hbox({window(
                    text(name) | color(Color::RGB(0, 0, 255)),
                    child->Render()
                    ), filler()});
            });
            children.push_back(wrapped);
        }
    }

    return Container::Vertical(std::move(children));
}


} // namespace smu
