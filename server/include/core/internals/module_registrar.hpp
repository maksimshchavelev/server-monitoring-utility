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
 * @brief The StringWrapper class to wrap `const char*`
 *
 * It is used to pass `const char*` to template parameters, as it is **not possible** to do so
 * otherwise.
 *
 * @tparam N Length of string
 *
 * @section example_usage Example usage
 * @code{.cpp}
 * template <StringWrapper string>
 * struct ExampleStruct {
 *      ExampleStruct() {
 *          std::cout << string.str << std::endl;
 *      }
 * }
 *
 * int main(int, char**) {
 *      ExampleStruct<StringWrapper{"Hello world!"}> es;
 *
 *      return 0;
 * }
 * @endcode
 *
 * Output:
 * ```
 * Hello world!
 * ```
 *
 */
template <std::size_t N> class StringWrapper {
  public:
    char str[N]; ///< This is where the string is stored.

    /**
     * @brief Constructor copies `s` to `str`
     * @param s Array of `const char`
     *
     * @note To access the string, refer to the `str` field.
     */
    consteval StringWrapper(const char (&s)[N]) {
        for (std::size_t i = 0; i < N; ++i) {
            str[i] = s[i];
        }
    }
};




/**
 * @brief Searches for a substring in a string regardless of case
 * @param str String
 * @param substr Substring
 * @return `true` if found, otherwise `false`
 *
 * @note The function can work at compile time
 *
 * @section example_usage Example usage
 * @code{.cpp}
 * // This is true
 * if constexpr (contains_substring("hello world", "WoRlD")) {
 *      std::cout << "Contains WoRlD!" << std::endl;
 * }
 *
 * // This is false
 * if constexpr (contains_substring("hello world", "abcd")) {
 *     std::cout << "Contains abcd!" << std::endl;
 * } else {
 *     std::cout << "Not contains abcd!" << std::endl;
 * }
 * @endcode
 *
 * Output:
 * ```
 * Contains WoRlD!
 * Not contains abcd!
 * ```
 */
consteval bool contains_substring(const char* str, const char* substr) {
    for (std::size_t i = 0; str[i]; ++i) {
        bool match = true;
        for (std::size_t j = 0; substr[j]; ++j) {
            char char_str = str[i + j];
            char char_substr = substr[j];

            if (static_cast<int>(char_str) >= 65 && static_cast<int>(char_str) <= 90) {
                // make lower
                char_str += 32;
            }

            if (static_cast<int>(char_substr) >= 65 && static_cast<int>(char_substr) <= 90) {
                // make lower
                char_substr += 32;
            }

            if (char_str != char_substr) {
                match = false;
                break;
            }
        }
        if (match) {
            return true;
        }
    }
    return false;
}




/**
 * @brief Supporting structure for module registration
 *
 * Its object can be created statically, which will result in automatic registration. This is
 * implemented in the `REGISTER_MODULE` macro.
 *
 * @tparam ModuleType Raw module type
 * @tparam module_name Name of module
 */
template <typename ModuleType, StringWrapper module_name> struct ModuleRegistrar {
    /**
     * @brief Registration constructor
     *
     * Adds a module to the registration queue by calling
     * `smu_server::Application::add_module_to_queue`
     *
     * @note The module must inherit from `smu_server::IModule` and not contain the word "module" in
     * any case in the module name.
     */
    ModuleRegistrar() {
        static_assert(!(contains_substring(module_name.str, "module")),
                      "Module name must not contain the word 'module'");
        static_assert(std::is_base_of_v<IModule, ModuleType>,
                      "The module must inherit from the IModule class");
        // Lazy module registering. Only adding to queue
        Application::instance().add_module_to_queue(
            [](Application& app) { app.register_module<ModuleType>(); });
    }
};

} // namespace internals



/// Write inside module class
#define REGISTER_MODULE(ModuleType, Description)                                                   \
  private:                                                                                         \
    inline static ::smu_server::internals::                                                        \
        ModuleRegistrar<ModuleType, ::smu_server::internals::StringWrapper{#ModuleType}>           \
                                              m_module_registrar;                                  \
    static constexpr internals::StringWrapper m_module_name{#ModuleType};                          \
    static constexpr internals::StringWrapper m_module_description{Description};                   \
                                                                                                   \
  public:                                                                                          \
    constexpr std::string_view module_name() const noexcept override {                             \
        return m_module_name.str;                                                                  \
    }                                                                                              \
                                                                                                   \
    constexpr std::string_view module_description() const noexcept override {                      \
        return m_module_description.str;                                                           \
    }                                                                                              \
                                                                                                   \
    constexpr static std::string_view module_name_static() noexcept {                              \
        return m_module_name.str;                                                                  \
    }                                                                                              \
                                                                                                   \
    constexpr static std::string_view module_description_static() noexcept {                       \
        return m_module_description.str;                                                           \
    }



} // end of namespace smu_server
