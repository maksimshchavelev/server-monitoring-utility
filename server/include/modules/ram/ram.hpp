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
 */
class RAM : public IModule {
public:
    REGISTER_MODULE(RAM, "A module that allows you to get information about RAM")
};


} // end of namespace smu_server
