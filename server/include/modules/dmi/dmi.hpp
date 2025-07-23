/// GPLv3 LICENSE, Copyright (©) 2025, Maksim Shchavelev <maksimshchavelev@gmail.com>
/// See LICENSE for details

/**
 * @file modules/dmi/dmi.hpp
 * @brief This module allows you to get data from DMI/SMBIOS tables. This is the data that the
 * manufacturer writes to the firmware (usually BIOS/UEFI), and it does not change when rebooting.
 */

#pragma once

#include "core/core.hpp"

namespace smu_server {

/**
 * @brief A module to get data from DMI/SMBIOS tables
 * @see `IModule` for methods
 */
class DMI : public IModule {
  public:
    REGISTER_MODULE(DMI, "Module for obtaining data from DMI/SMBIOS tables")




    DMI(const Json::Value& config);




    std::optional<Json::Value> get_data() override;


  private:
    // BIOS
    std::string m_bios_date;
    std::string m_bios_vendor;
    std::string m_bios_version;

    // BOARD
    std::string m_board_name;
    std::string m_board_vendor;
    std::string m_board_version;

    // CHASSIS
    std::string m_chassis_vendor;
    std::string m_chassis_type;

    // PRODUCT
    std::string m_product_family;
    std::string m_product_name;
    std::string m_product_serial;
    std::string m_product_uuid;


    // To avoid regular construction of an object from static data
    std::unique_ptr<IMetricNodeBase> m_root;



    /**
     * @brief Reads the file
     * @param path Path to file
     * @return `std::expected` with `std::string` with data if success, otherwise `std::string` with
     * error description
     */
    std::expected<std::string, std::string> read_file(const std::string_view path) const noexcept;




    /**
     * @brief Fills bios info
     * @note Prints log if error
     */
    void fill_bios_info();




    /**
     * @brief Fills board info
     * @note Prints log if error
     */
    void fill_board_info();




    /**
     * @brief Fills chassis info
     * @note Prints log if error
     */
    void fill_chassis_info();




    /**
     * @brief Fills product info
     * @note Prints log if error
     */
    void fill_product_info();
};

} // namespace smu_server
