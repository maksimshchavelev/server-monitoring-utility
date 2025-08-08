/// GPLv3 LICENSE, Copyright (©) 2025, Maksim Shchavelev <maksimshchavelev@gmail.com>
/// See LICENSE for details

/**
 * @file modules/ui/ui.hpp
 * @brief File with user interface
 */

#pragma once

#include "internals/tabs.hpp"
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <json/json.h>

namespace smu {

/**
 * @brief Class with user interface
 */
class UI {
  public:
    /**
     * @brief UI constructor
     */
    UI();




    /**
     * @brief Runs UI async
     */
    void run_async();




    /**
     * @brief Stop UI
     */
    void stop();




    /**
     * @brief Set json data to draw
     * @note The data must be obtained from **smu-server**
     * @param data Data
     */
    void set_data(const Json::Value& data);


  private:
    std::mutex               m_data_mutex; ///< Mutex to prevent data rave with m_data
    Json::Value              m_data{};     ///< Json data
    ftxui::ScreenInteractive m_screen;     ///< Screen object

    std::shared_ptr<Tabs> m_tabs; ///< Tabs to render

    /**
     * @brief Unwraps json representation of module to UI
     * @param data Json data
     * @param path Path for current node
     * @return `ftxui::Element` with UI
     * @note Call for each module
     */
    ftxui::Component unwrap_module(const Json::Value& data);
};


} // namespace smu
