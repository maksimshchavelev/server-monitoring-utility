/// GPLv3 LICENSE, Copyright (©) 2025, Maksim Shchavelev <maksimshchavelev@gmail.com>
/// See LICENSE for details

/**
 * @file modules/ui/internals/tabs.hpp
 * @brief Tabs UI interactive component
 */

#pragma once

#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <vector>

namespace smu {

/**
 * @brief Tabs UI interactive component
 */
class Tabs : public ftxui::ComponentBase {
  public:
    /**
     * @brief Tabs constructor
     * @param headers Names of headers
     * @param tabs Content of tabs
     */
    Tabs(const std::vector<ftxui::Element>& headers, const std::vector<ftxui::Component>& tabs);




    /**
     * @brief OnRender Draw the component. Build a ftxui::Element to be drawn on the ftxi::Screen
     * representing this ftxui::ComponentBase
     * @return `ftxui::Element`
     */
    ftxui::Element OnRender() override;




    /**
     * @brief Called in response to an event.
     * @param e Event
     * @return `true` if event is catched, else `false`
     */
    bool OnEvent(ftxui::Event e) override;



  private:
    std::vector<ftxui::Element>   m_headers;
    std::vector<ftxui::Component> m_tabs; // components to render in each tab

    std::size_t m_current_tab{0};
    std::size_t m_tabs_count{0};
};




/**
 * @brief Makes `Tabs` component
 * @param headers Tabs headers
 * @param tabs Tabs content
 * @return `ftxui::Component`
 * @see `Tabs`
 */
ftxui::Component make_tabs(const std::vector<ftxui::Element>&   headers,
                           const std::vector<ftxui::Component>& tabs);


} // namespace smu
