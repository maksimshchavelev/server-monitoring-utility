/// GPLv3 LICENSE, Copyright (©) 2025, Maksim Shchavelev <maksimshchavelev@gmail.com>
/// See LICENSE for details

/**
 * @file modules/dmi/dmi.hpp
 * @brief This module allows you to get data from DMI/SMBIOS tables. This is the data that the
 * manufacturer writes to the firmware (usually BIOS/UEFI), and it does not change when rebooting.
 */

#include "modules/dmi/dmi.hpp"
#include <fstream>

namespace smu_server {

// Public constructor
DMI::DMI(const Config& configuration) : IModule(configuration) {
    // Create new configuration
    if (m_configuration.empty()) {
        m_configuration.set("enabled", true);

        // Cache data and prevent subsequent calls to `get_data`
        m_configuration.set("poll_ratio", 0);
    }

    // Check if the `enabled` field exists. If not, log the error and throw an exception to abort
    // registration.
    if (auto enabled = m_configuration.get<bool>("enabled"); enabled.has_value()) {
        m_enabled = enabled.value();
    } else {
        log(LogType::ERROR, "The 'enabled' field is missing. Can't continue");
        throw std::runtime_error("The 'enabled' field is missing");
    }

    // Check if the `poll_ratio` field exists. If not, log the error and throw an exception to abort
    // registration.
    if (auto poll_ratio = m_configuration.get<unsigned int>("poll_ratio"); poll_ratio.has_value()) {
        m_poll_ratio = poll_ratio.value();
    } else {
        log(LogType::ERROR, "The 'poll_ratio' field is missing. Can't continue");
        throw std::runtime_error("The 'poll_ratio' field is missing");
    }

    // Collect information
    fill_bios_info();
    fill_board_info();
    fill_chassis_info();
    fill_product_info();
}




// Public method
std::optional<std::vector<uint8_t>> DMI::get_data() {
    auto root = make_root_node(
        // BIOS INFO
        make_container_node("BIOS",
                            make_value_node("BIOS date", m_bios_date, ""),
                            make_value_node("BIOS vendor", m_bios_vendor, ""),
                            make_value_node("BIOS version", m_bios_version, "")),

        // BOARD INFO
        make_container_node("Board",
                            make_value_node("Board name", m_board_name, ""),
                            make_value_node("Board vendor", m_board_vendor, ""),
                            make_value_node("Board version", m_board_version, "")),

        // CHASSIS INFO
        make_container_node("Chassis",
                            make_value_node("Chassis type", m_chassis_type, ""),
                            make_value_node("Chassis vendor", m_chassis_vendor, "")),

        // PRODUCT INFO
        make_container_node("Product",
                            make_value_node("Product name", m_product_name, ""),
                            make_value_node("Product family", m_product_family, ""),
                            make_value_node("Product serial", m_product_serial, ""),
                            make_value_node("Product UUID", m_product_uuid, "")));

    return root->to_mdtp();
}




// Public method
void DMI::enable() {
    m_configuration.set("enabled", true);
    m_enabled = true;
}




// Public method
void DMI::disable() {
    m_configuration.set("enabled", false);
    m_enabled = false;
}




// Private method
std::expected<std::string, std::string> DMI::read_file(const std::string_view path) const noexcept {
    std::ifstream file;
    file.open(path.data());

    if (!file.is_open()) {
        // If fail
        return std::unexpected(
            std::format("Error opening file with name {}, cause: {}", path, strerror(errno)));
    }

    std::string data((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    file.close();

    return data;
}




// Private method
void DMI::fill_bios_info() {
    const std::string path = "/sys/class/dmi/id/";

    if (auto data = read_file(path + "bios_date"); data.has_value()) {
        m_bios_date = std::move(data.value());
    } else {
        log(LogType::WARNING, data.error());
    }

    if (auto data = read_file(path + "bios_vendor"); data.has_value()) {
        m_bios_vendor = std::move(data.value());
    } else {
        log(LogType::WARNING, data.error());
    }

    if (auto data = read_file(path + "bios_version"); data.has_value()) {
        m_bios_version = std::move(data.value());
    } else {
        log(LogType::WARNING, data.error());
    }
}




// Private method
void DMI::fill_board_info() {
    const std::string path = "/sys/class/dmi/id/";

    if (auto data = read_file(path + "board_name"); data.has_value()) {
        m_board_name = std::move(data.value());
    } else {
        log(LogType::WARNING, data.error());
    }

    if (auto data = read_file(path + "board_vendor"); data.has_value()) {
        m_board_vendor = std::move(data.value());
    } else {
        log(LogType::WARNING, data.error());
    }

    if (auto data = read_file(path + "board_version"); data.has_value()) {
        m_board_version = std::move(data.value());
    } else {
        log(LogType::WARNING, data.error());
    }
}




// Private method
void DMI::fill_chassis_info() {
    const std::string path = "/sys/class/dmi/id/";

    if (auto data = read_file(path + "chassis_type"); data.has_value()) {
        m_chassis_type = std::move(data.value());
    } else {
        log(LogType::WARNING, data.error());
    }

    if (auto data = read_file(path + "chassis_vendor"); data.has_value()) {
        m_chassis_vendor = std::move(data.value());
    } else {
        log(LogType::WARNING, data.error());
    }
}




// Private method
void DMI::fill_product_info() {
    const std::string path = "/sys/class/dmi/id/";

    if (auto data = read_file(path + "product_family"); data.has_value()) {
        m_product_family = std::move(data.value());
    } else {
        log(LogType::WARNING, data.error());
    }

    if (auto data = read_file(path + "product_name"); data.has_value()) {
        m_product_name = std::move(data.value());
    } else {
        log(LogType::WARNING, data.error());
    }

    if (auto data = read_file(path + "product_serial"); data.has_value()) {
        m_product_serial = std::move(data.value());
    } else {
        log(LogType::WARNING, data.error());
    }

    if (auto data = read_file(path + "product_uuid"); data.has_value()) {
        m_product_uuid = std::move(data.value());
    } else {
        log(LogType::WARNING, data.error());
    }
}



} // namespace smu_server
