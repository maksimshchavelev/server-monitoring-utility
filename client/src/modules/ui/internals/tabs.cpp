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

    if (m_tabs.empty())
        return text("Nothing to render");

    int         screen_size_y = Terminal::Size().dimy;
    const auto& selected_content = m_tabs[m_current_tab];

    // Build headers dynamically with focus/selection styling.
    std::vector<Element> visible_headers;
    for (std::size_t i = m_first_visible_header;
         i < std::min(m_last_visible_header + 1, m_tabs_count);
         ++i) {
        if (m_focus_on_content) {
            visible_headers.push_back(m_headers[i] | dim);
        } else {
            visible_headers.push_back(m_headers[i] | bold);
        }
    }

    auto headers_pane =
        vbox(std::move(visible_headers)) | size(HEIGHT, Constraint::EQUAL, screen_size_y);

    // Give a subtle focus hint to the content pane.
    auto content_el = selected_content->Render();

    return hbox(headers_pane, separator(), content_el) | borderRounded;
}




// Public method
bool Tabs::OnEvent(ftxui::Event e) {
    using namespace ftxui;

    std::size_t screen_height = static_cast<std::size_t>(Terminal::Size().dimy);

    // ---- Left/Right switch focus between headers and content ----
    if (e == Event::ArrowRight) {
        m_focus_on_content = true;
        return true;
    }
    if (e == Event::ArrowLeft) {
        if (m_focus_on_content) {
            m_focus_on_content = false;
            return true;
        }
        // If focus is already on headers, keep processing below.
    }

    // ---- When focus is on headers: Up/Down navigate headers ----
    if (!m_focus_on_content) {
        if (e == Event::ArrowDown) {
            if (m_current_tab + 1 < m_tabs_count) {
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
        if (e == Event::ArrowUp) {
            if (m_current_tab > 0) {
                m_headers[m_current_tab] |= inverted;
                --m_current_tab;
                m_headers[m_current_tab] |= inverted;
                if (screen_height - 2 < m_headers.size()) {
                    --m_first_visible_header;
                    --m_last_visible_header;
                } else if (m_first_visible_header > 0) {
                    --m_first_visible_header;
                }
            }
            return true;
        }
        // Other keys not handled by headers.
        return false;
    }

    // ---- When focus is on content: forward events to active tab ----
    if (m_tabs.empty()) {
        return false;
    }

    if (m_tabs[m_current_tab]->OnEvent(e)) {
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

    // Keep current focus side (content vs headers) as-is on data refresh.
}




// External method
std::shared_ptr<Tabs> make_tabs(const std::vector<ftxui::Element>&   headers,
                                const std::vector<ftxui::Component>& tabs) {
    return std::make_shared<Tabs>(headers, tabs);
}




// External method
std::shared_ptr<Tabs> make_tabs() {
    return std::make_shared<Tabs>();
}




} // namespace smu
