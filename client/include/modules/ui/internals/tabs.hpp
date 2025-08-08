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
     * @note The number of elements in `headers` must be equal to the number of elements in `tabs`,
     * otherwise an exception is thrown.
     */
    Tabs(const std::vector<ftxui::Element>& headers, const std::vector<ftxui::Component>& tabs);




    /**
     * @brief Default tabs constructor. Does nothing
     */
    Tabs() = default;




    /**
     * @brief Draws the component. Build a ftxui::Element to be drawn on the ftxi::Screen
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
     */
    void set_data(std::vector<ftxui::Element>&& headers, std::vector<ftxui::Component>&& content);



  private:
    std::vector<ftxui::Element>   m_headers; ///< Headers of each tab
    std::vector<ftxui::Component> m_tabs;    ///< Components to render in each tab

    std::size_t m_current_tab{0}; ///< Current tab index
    std::size_t m_tabs_count{0};  ///< Count of tabs

    std::size_t m_first_visible_header{0}; ///< First visible header in visible headers interval
    std::size_t m_last_visible_header{0};  ///< Last visible header in visible headers interval
};




/**
 * @brief Makes `Tabs` component
 * @param headers Tabs headers
 * @param tabs Tabs content
 * @return `std::shared_ptr<Tabs>`
 * @see `Tabs`
 */
std::shared_ptr<Tabs> make_tabs(const std::vector<ftxui::Element>&   headers,
                                const std::vector<ftxui::Component>& tabs);




/**
 * @brief Makes empty `Tabs` component
 * @return `std::shared_ptr<Tabs>`
 * @see `Tabs`
 */
std::shared_ptr<Tabs> make_tabs();


} // namespace smu
