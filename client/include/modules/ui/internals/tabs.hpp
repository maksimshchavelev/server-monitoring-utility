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
     * @brief Default tabs constructor
     */
    Tabs() = default;




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




    /**
     * @brief Sets new tabs and their contents
     * @param headers Tab headers
     * @param content Tab contents
     * @throw Throws an exception if the number of headers is not equal to the number of tabs
     * @note Use via `dynamic_cast`
     */
    void set_data(std::vector<ftxui::Element>&& headers, std::vector<ftxui::Component>&& content);



  private:
    std::vector<ftxui::Element>   m_headers;
    std::vector<ftxui::Component> m_tabs; // components to render in each tab

    std::size_t m_current_tab{0};
    std::size_t m_tabs_count{0};

    std::size_t m_first_visible_header{0};
    std::size_t m_last_visible_header{0};
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




/**
 * @brief Makes empty `Tabs` component
 * @return `ftxui::Component`
 * @see `Tabs`
 */
ftxui::Component make_tabs();


} // namespace smu
