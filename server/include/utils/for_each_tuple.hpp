/// GPLv3 LICENSE, Copyright (©) 2025, Maksim Shchavelev <maksimshchavelev@gmail.com>
/// See LICENSE for details

/**
 * @file utils/for_each_tuple.hpp
 * @brief File with for_each_tuple implementation
 */

#pragma once

#include <tuple>

namespace smu_server {

namespace internals {

/**
 * @brief Implementation of for_each_tuple.
 * @note Do not use directly
 * @param tuple std::tuple
 * @param function Callback to which each tuple element is passed by universal reference
 * @see `for_each_tuple`
 */
template <typename Tuple, typename Function, std::size_t... Indexes>
constexpr void for_each_tuple_impl(Tuple&&    tuple,
                                   Function&& function,
                                   std::index_sequence<Indexes...>) {
    (function(std::get<Indexes>(std::forward<Tuple>(tuple))), ...);
}

}


/**
 * @brief Iterates over the tuple elements and passes each element to the `function` function
 * @param tuple `std::tuple`
 * @param function Function to which each tuple element is passed by universal reference
 * @example
 *
 * ```
 * auto t = std::make_tuple(42, 3.14, std::string("Hello"));
 *
 * for_each_tuple(t, [](const auto& element) {
 *      std::cout << element << '\n'; // Print each tuple element
 * });
 * ```
 */
template <typename Tuple, typename Function>
constexpr void for_each_tuple(Tuple&& tuple, Function&& function) {
    for_each_tuple_impl(std::forward<Tuple>(tuple),
                        std::forward<Function>(function),
                        std::make_index_sequence<std::tuple_size_v<std::decay_t<Tuple>>>());
}

}
