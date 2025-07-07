/// GPLv3 LICENSE, Copyright (©) 2025, Maksim Shchavelev <maksimshchavelev@gmail.com>
/// See LICENSE for details

/**
 * @file core/internals/register_module.hpp
 * @brief File with module registration utils
 */


#pragma once

#include "module.hpp"
#include <cstddef>
#include <type_traits>

namespace smu_server {

namespace internals {

/**
 * @brief Supporting structure for module registration
 */
template <typename ModuleName, const char* module_name> struct ModuleRegistrar {
    /**
     * @brief Constructor for registration
     */
    ModuleRegistrar() {
        static_assert(!(contains_substring(module_name, "module")),
                      "Module name must not contain the word 'module'");
        static_assert(!std::is_base_of_v<Module, ModuleName>,
                      "The module must inherit from the IModule class");
        /// TODO: make ::smu_server::app().register_module(...)
    }

  private:
    /**
     * @brief Searches for a substring in a string regardless of case
     * @param str String
     * @param substr Substring
     * @return `true` if found, otherwise `false`
     */
    consteval bool contains_substring(const char* str, const char* substr) {
        for (std::size_t i = 0; str[i]; ++i) {
            bool match = true;
            for (std::size_t j = 0; substr[j]; ++j) {
                char char_str = str[i + j];
                char char_substr = str[j];

                if (static_cast<int>(char_str) >= 65 && static_cast<int>(char_str) <= 90) {
                    // make lower
                    char_str += 32;
                }

                if (static_cast<int>(char_substr) >= 65 && static_cast<int>(char_substr) <= 90) {
                    // make lower
                    char_substr += 32;
                }

                if (str[i + j] != substr[j]) {
                    match = false;
                    break;
                }
            }
            if (match) {
                return true;
            }
        };
    }
};

} // namespace internals



/// Write inside module class
#define REGISTER_MODULE(module, description)                                                       \
  private:                                                                                         \
    [[no_unique_address]] static ::smu_server::internals::ModuleRegistrar<module, #module>         \
               m_module_registrar;                                                                 \
    const char m_module_name[] = #module;                                                          \
    const char m_module_description[] = #description;                                              \
                                                                                                   \
  public:



} // end of namespace smu_server
