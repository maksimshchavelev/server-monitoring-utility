/// GPLv3 LICENSE, Copyright (©) 2025, Maksim Shchavelev <maksimshchavelev@gmail.com>
/// See LICENSE for details

/**
 * @file modules/ui/internals/tabs.cpp
 * @brief Tabs UI interactive component
 */

#include "modules/ui/internals/tabs.hpp"

namespace smu {

// Public constructor
Tabs::Tabs(const std::vector<ftxui::Element>& headers, const std::vector<ftxui::Component>& tabs) :
    m_headers(headers), m_tabs(tabs) {
    if (m_headers.size() != m_tabs.size()) {
        throw std::runtime_error("Headers count must be equal tabs count");
    }

    m_tabs_count = m_headers.size();
    m_headers[m_current_tab] |= ftxui::inverted;

    m_last_visible_header =
        std::min({m_tabs_count, static_cast<std::size_t>(ftxui::Terminal::Size().dimy - 2)});
}




// Public method
ftxui::Element Tabs::OnRender() {
    using namespace ftxui;

    // Prevent crash. We can't get a reference to a non-existent element if m_tabs.size() == 0
    if (m_tabs.empty())
        return text("Nothing to render");

    // else

    int screen_size_y = Terminal::Size().dimy;

    const auto& selected_content = m_tabs[m_current_tab];

    std::vector<Element> visible_headers;
    for (std::size_t i = m_first_visible_header;
         i < std::min(m_last_visible_header + 1, m_tabs_count);
         ++i) {
        visible_headers.push_back(m_headers[i]);
    }

    return hbox(vbox(visible_headers) | size(HEIGHT, Constraint::EQUAL, screen_size_y),
                separator(),
                selected_content->Render()) |
           borderRounded;
}




// Public method
bool Tabs::OnEvent(ftxui::Event e) {
    using namespace ftxui;

    std::size_t screen_height = static_cast<std::size_t>(Terminal::Size().dimy);

    // Pressed arrow down
    if (e == Event::ArrowDown) {
        if (m_current_tab < m_tabs_count - 1) {
            m_headers[m_current_tab] |= inverted;
            ++m_current_tab;
            m_headers[m_current_tab] |= inverted;

            if (screen_height - 2 < m_headers.size()) {
                ++m_first_visible_header;
                ++m_last_visible_header;
            }
        }
        return true;
    }
    // Pressed arrow up
    else if (e == Event::ArrowUp) {
        if (m_current_tab > 0) {
            m_headers[m_current_tab] |= inverted;
            --m_current_tab;
            m_headers[m_current_tab] |= inverted;

            if (screen_height - 2 < m_headers.size()) {
                --m_first_visible_header;
                --m_last_visible_header;
            } else {
                if (m_first_visible_header > 0) {
                    --m_first_visible_header;
                }
            }
        }
        return true;
    }

    return false;
}




// Public method
void Tabs::set_data(std::vector<ftxui::Element>&&   headers,
                    std::vector<ftxui::Component>&& content) {
    if (headers.size() != content.size()) {
        throw std::runtime_error("Headers count must be equal tabs count");
    }

    m_headers = std::move(headers);
    m_tabs = std::move(content);

    m_tabs_count = m_headers.size();
    m_headers[m_current_tab] |= ftxui::inverted;

    m_last_visible_header =
        std::min({m_tabs_count, static_cast<std::size_t>(ftxui::Terminal::Size().dimy - 2)});
}




// External method
ftxui::Component make_tabs(const std::vector<ftxui::Element>&   headers,
                           const std::vector<ftxui::Component>& tabs) {
    return std::make_shared<Tabs>(headers, tabs);
}




} // namespace smu
