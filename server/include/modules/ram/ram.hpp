/// GPLv3 LICENSE, Copyright (©) 2025, Maksim Shchavelev <maksimshchavelev@gmail.com>
/// See LICENSE for details

/**
 * @file modules/ram/ram.hpp
 * @brief File with RAM module class
 */

#pragma once

#include <core/core.hpp> // For server core utils
#include <unordered_map> // For std::unordered_map

namespace smu_server {


/**
 * @brief A module that allows you to get information about RAM
 * @see `IModule` for methods
 */
class RAM : public IModule {
  public:
    REGISTER_MODULE(RAM, "A module that allows you to get information about RAM")

    RAM(const Config& config);

    std::optional<std::vector<uint8_t>> get_data() override;

    void enable() override;

    void disable() override;

  private:
    /**
     * @brief The function parses `/proc/meminfo` to obtain memory information. It returns data in
     * the form of a hash table.
     *
     * For example, if you need to get the value of the `MemTotal` parameter, you should write code
     * similar to the following:
     *
     * @code{.cpp}
     * auto meminfo = parse_meminfo();
     *
     * // Make sure that the MemTotal key is actually present!
     * std::cout << meminfo["MemTotal"];
     * @endcode
     *
     * @param path Path to `meminfo`
     *
     * @throws Throws a `std::runtime_error` exception when an error occurs (with an error description)
     *
     * @return `std::unordered_map` with data. **All values are in bytes**
     */
    std::unordered_map<std::string /* parameter */, std::size_t /* value in bytes */> parse_meminfo(
        const std::string_view path) const;

    std::string m_meminfo_path; ///< Path to the meminfo file. /proc/meminfo by default
};


} // end of namespace smu_server
