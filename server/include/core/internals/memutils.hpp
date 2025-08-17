/**
 * @file core/internals/memutils.hpp
 * @brief File with memory utils
 *
 * @copyright Copyright (©) 2025, Maksim Shchavelev <maksimshchavelev@gmail.com>
 * @license GPLv3 license, see LICENSE for details
 */

#pragma once

#include <cstdint>
#include <span>

namespace smu_server::internals {

/**
 * @brief Writes a signed 32-bit integer to `memory` starting at offset `offset` in **Big Endian
 * order**
 *
 * @section example_usage Example usage
 * After executing this code
 * @code{.cpp}
 * write_uint32_be(mem, 0, 12)
 * @endcode
 *
 * Memory `mem` will look like:
 * ```
 * [0] : 0x00
 * [1] : 0x00
 * [2] : 0x00
 * [3] : 0x0C (12)
 * ```
 *
 * @param memory Memory
 * @param offset Offset
 * @param value Value to write
 */
inline void write_uint32_be(std::span<uint8_t> memory, std::size_t offset, uint32_t value) {
    memory[offset] = (value >> 24) & 0xFF;
    memory[offset + 1] = (value >> 16) & 0xFF;
    memory[offset + 2] = (value >> 8) & 0xFF;
    memory[offset + 3] = value & 0xFF;
}




/**
 * @brief Writes an unsigned 8-bit integer (a byte) to `memory`.
 * @details Endianness does not apply to a single byte, but the function is provided
 * for a consistent API.
 *
 * @section example_usage Example usage
 * After executing this code:
 * @code{.cpp}
 * write_ubyte_be(mem, 0, 255);
 * @endcode
 *
 * Memory `mem` will look like:
 * ```
 * [0] : 0xFF (255)
 * ```
 *
 * @param memory The memory buffer to write to.
 * @param offset The offset in bytes from the start of the buffer.
 * @param value The 8-bit value to write.
 */
inline void write_ubyte_be(std::span<uint8_t> memory, std::size_t offset, uint8_t value) {
    memory[offset] = value;
}




/**
 * @brief Reads an unsigned 8-bit integer (a byte) from `memory`.
 *
 * @section example_usage Example usage
 * If memory `mem` looks like this:
 * ```
 * [0] : 0xFF (255)
 * ```
 * Then the code:
 * @code{.cpp}
 * uint8_t val = read_ubyte_be(mem, 0); // val will be 255
 * @endcode
 *
 * @param memory The memory buffer to read from.
 * @param offset The offset in bytes from the start of the buffer.
 * @return The 8-bit value read from memory.
 */
inline uint8_t read_ubyte_be(std::span<const uint8_t> memory, std::size_t offset) {
    return memory[offset];
}




/**
 * @brief Reads an unsigned 32-bit integer from `memory` in **Big Endian** order.
 *
 * @section example_usage Example usage
 * If memory `mem` looks like this:
 * ```
 * [0] : 0x00 (MSB)
 * [1] : 0x00
 * [2] : 0x00
 * [3] : 0x0C (LSB)
 * ```
 * Then the code:
 * @code{.cpp}
 * uint32_t val = read_uint32_be(mem, 0); // val will be 12
 * @endcode
 *
 * @param memory The memory buffer to read from.
 * @param offset The offset in bytes from the start of the buffer.
 * @return The 32-bit value reconstructed from memory.
 */
inline uint32_t read_uint32_be(std::span<const uint8_t> memory, std::size_t offset) {
    return (static_cast<uint32_t>(memory[offset]) << 24) |
           (static_cast<uint32_t>(memory[offset + 1]) << 16) |
           (static_cast<uint32_t>(memory[offset + 2]) << 8) |
           (static_cast<uint32_t>(memory[offset + 3]));
}


} // namespace smu_server::internals
