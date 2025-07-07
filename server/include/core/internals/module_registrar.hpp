/// GPLv3 LICENSE, Copyright (©) 2025, Maksim Shchavelev <maksimshchavelev@gmail.com>
/// See LICENSE for details

/**
 * @file core/internals/register_module.hpp
 * @brief File with module registration utils
 */


#pragma once

#include "app.hpp"
#include "module.hpp"
#include <cstddef>
#include <type_traits>

namespace smu_server {

namespace internals {

/**
 * @brief The StringWrapper class to wrap const char* in compile-time
 */
template <std::size_t N> class StringWrapper {
  public:
    char str[N];

    /**
     * @brief Copies `s` to `str`
     */
    consteval StringWrapper(const char (&s)[N]) {
        for (std::size_t i = 0; i < N; ++i) {
            str[i] == s[i];
        }
    }
};




/**
 * @brief Supporting structure for module registration
 */
template <typename ModuleName, StringWrapper module_name> struct ModuleRegistrar {
    /**
     * @brief Constructor for registration
     */
    ModuleRegistrar() {
        static_assert(!(contains_substring(module_name.str, "module")),
                      "Module name must not contain the word 'module'");
        static_assert(!std::is_base_of_v<IModule, ModuleName>,
                      "The module must inherit from the IModule class");
        Application::instance().register_module<ModuleName>();
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
#define REGISTER_MODULE(ModuleType, Description)                                                   \
  private:                                                                                         \
    static ::smu_server::internals::                                                               \
        ModuleRegistrar<ModuleType, ::smu_server::internals::StringWrapper(#ModuleType)>           \
               m_module_registrar;                                                                 \
    const char m_module_name[] = #ModuleType;                                                      \
    const char m_module_description[] = #Description;                                              \
                                                                                                   \
  public:



} // end of namespace smu_server
