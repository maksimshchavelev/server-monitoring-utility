/// GPLv3 LICENSE, Copyright (©) 2025, Maksim Shchavelev <maksimshchavelev@gmail.com>
/// See LICENSE for details

/**
 * @file modules/ram/ram.cpp
 * @brief File with RAM module class
 */


#include "modules/ram/ram.hpp"


namespace smu_server {


// Public method
RAM::RAM(const Config& config) : IModule(config) {
    // Create new configuration
    if (m_configuration.empty()) {
        m_configuration.set("enabled", true);
        m_configuration.set("poll_ratio", 1);
        m_configuration.set("meminfo_path", "/proc/meminfo");
    }

    // Check if the `enabled` field exists. If not, log the error and throw an exception to abort
    // registration.
    if (auto enabled = m_configuration.get<bool>("enabled"); enabled.has_value()) {
        m_enabled = enabled.value();
    } else {
        throw std::runtime_error("The 'enabled' field in the configuration is missing");
    }

    // Check if the `poll_ratio` field exists. If not, log the error and throw an exception to abort
    // registration.
    if (auto poll_ratio = m_configuration.get<unsigned int>("poll_ratio"); poll_ratio.has_value()) {
        m_poll_ratio = poll_ratio.value();
    } else {
        throw std::runtime_error("The 'poll_ratio' field in the configuration is missing");
    }

    // Check if the `meminfo_path` field exists. If not, log the error and throw an exception to abort
    // registration.
    if (auto meminfo_path = m_configuration.get<std::string>("meminfo_path"); meminfo_path.has_value()) {
        m_meminfo_path = meminfo_path.value();
    } else {
        throw std::runtime_error("The 'meminfo_path' field in the configuration is missing");
    }

    // Try to parse meminfo and print log
    log(LogType::INFO, std::format("Trying to parse `{}`...", m_meminfo_path));
    for (const auto& [key, value] : parse_meminfo(m_meminfo_path)) {
        log(LogType::INFO, std::format("{}: {} kbytes", key, value / 1024));
    }
}


// Public method
std::optional<std::vector<uint8_t>> RAM::get_data() {
    std::unordered_map<std::string, std::size_t> meminfo;

    try {
        meminfo = parse_meminfo(m_meminfo_path);
    } catch (const std::runtime_error& e) {
        log(LogType::ERROR, e.what());
        return std::nullopt;
    }

    // Ram info
    const std::size_t totalram_mb = meminfo["MemTotal"] / 1024 / 1024;
    const std::size_t usedram_mb = totalram_mb - meminfo["MemAvailable"] / 1024 / 1024;
    const std::size_t usedram_percents = usedram_mb * 100 / totalram_mb;

    // Swap info
    const std::size_t totalswap_mb = meminfo["SwapTotal"] / 1024 / 1024;
    const std::size_t usedswap_mb = totalswap_mb - meminfo["SwapFree"] / 1024 / 1024;
    const std::size_t usedswap_percents = usedswap_mb * 100 / totalswap_mb;


    auto root = make_root_node(make_container_node("RAM info",
                                                   make_value_node("Total RAM", totalram_mb, "MB"),
                                                   make_value_node("Used RAM", usedram_mb, "MB"),
                                                   make_value_node("Used RAM (%)", usedram_percents, "%")),

                               make_container_node("SWAP info",
                                                   make_value_node("Total SWAP", totalswap_mb, "MB"),
                                                   make_value_node("SWAP usage", usedswap_mb, "MB"),
                                                   make_value_node("SWAP usage (%)", usedswap_percents, "%")));

    return root->to_mdtp();
}


// Public method
void RAM::enable() {
    m_configuration.set("enabled", true);
    m_enabled = true;
}


// Public method
void RAM::disable() {
    m_configuration.set("enabled", false);
    m_enabled = false;
}


// Private method
std::unordered_map<std::string, std::size_t> RAM::parse_meminfo(const std::string_view path) const {
    auto meminfo_read_result = SystemFileReader::read_text_file(path);

    // Error occured
    if (!meminfo_read_result.has_value()) {
        // Allow the kernel to catch the exception itself and prevent the module from being
        // registered or sending module data to the user.
        throw std::runtime_error(std::format("error in method `parse_meminfo()`: `SystemFileReader::read_text_file` "
                                             "returned error code {} (description: {}) for file `{}`",
                                             static_cast<int>(meminfo_read_result.error()),
                                             SystemFileReader::error_description(meminfo_read_result.error()),
                                             path));
    }

    // Parse
    std::unordered_map<std::string, std::size_t> result;

    for (const auto& line : SystemFileReader::split_string(meminfo_read_result.value(), "\n")) {
        const std::string_view key = SystemFileReader::token_before(line, ":");

        // You need to parse the token after the colon and split it into the value and units of
        // measurement.
        const auto tokens_after_key =
            SystemFileReader::split_string(SystemFileReader::trim(SystemFileReader::token_after(line, ":")));

        // It's not the size, it's the quantity
        if (tokens_after_key.size() <= 1)
            continue;

        std::size_t value = SystemFileReader::convert_units(
            tokens_after_key[1], SystemFileReader::SizeUnit::BYTES, std::stoull(std::string(tokens_after_key[0])));

        result[std::string(key)] = value;
    }

    return result;
}

} // namespace smu_server
