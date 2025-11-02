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
    void set_data(const std::vector<uint8_t>& data);


  private:
    std::mutex m_data_mutex; ///< Mutex to prevent data rave with m_data
    std::unordered_map<std::string, std::vector<uint8_t>> m_data{}; ///< Modules data
    ftxui::ScreenInteractive                              m_screen; ///< Screen object

    std::shared_ptr<Tabs> m_tabs; ///< Tabs to render

    /**
     * @brief Unwraps json representation of module to UI
     * @param data MDTP data of module
     * @param path Path for current node
     * @return `ftxui::Element` with UI
     * @note Call for each module
     */
    ftxui::Component unwrap_module(const std::vector<uint8_t>& module_data);

    /**
     * @brief Decodes module names in the MDTP protocol
     * @param mdtp_data MDTP encoded bytes
     * @return `std::vector<std::string>` with module names
     */
    std::vector<std::string> get_module_names_from_mdtp(const std::vector<uint8_t>& mdtp_data);

    /**
     * @brief Reads an unsigned 32-bit integer from `memory` in **Big Endian** order.
     *
     * @section example_usage Example usage
     * If memory `mem` looks like this:
     * ```
     * [0] : 0x00 (MSB)
     * [1] : 0x00
     * [2] : 0x00
     * [3] : 0x0C (LSB)
     * ```
     * Then the code:
     * @code{.cpp}
     * uint32_t val = read_uint32_be(mem, 0); // val will be 12
     * @endcode
     *
     * @param memory The memory buffer to read from.
     * @param offset The offset in bytes from the start of the buffer.
     * @return The 32-bit value reconstructed from memory.
     */
    uint32_t read_uint32_be(std::span<const uint8_t> memory, std::size_t offset);
};


} // namespace smu
