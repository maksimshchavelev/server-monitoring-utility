/// GPLv3 LICENSE, Copyright (©) 2025, Maksim Shchavelev <maksimshchavelev@gmail.com>
/// See LICENSE for details

/**
 * @file modules/ram/ram.hpp
 * @brief File with RAM module class
 */

#pragma once

#include "core/core.hpp"

namespace smu_server {


/**
 * @brief A module that allows you to get information about RAM
 * @see `IModule` for methods
 */
class RAM : public IModule {
  public:
    REGISTER_MODULE(RAM, "A module that allows you to get information about RAM")




    RAM(const Json::Value& config);




    std::optional<Json::Value> get_data() override;




    void enable() override;




    void disable() override;
};


} // end of namespace smu_server
